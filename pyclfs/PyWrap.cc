#define PY_SSIZE_T_CLEAN
#include <stdexcept>
#include "PyWrap.h"
#include <string>
#include <map>
#include <sstream>
#include <boost/python/numpy.hpp>
#include <iostream>

namespace pywrap {
    namespace np = boost::python::numpy;
    PyWrap* PyWrap::wrapper = nullptr;
    std::mutex PyWrap::mutex;
    CPyInstance* PyWrap::pyInstance = nullptr;
    // moduleClassMap is now an instance member - removed global declaration

    PyWrap* PyWrap::GetInstance()
    {
        std::lock_guard<std::mutex> lock(mutex);
        if (wrapper == nullptr) {
            wrapper = new PyWrap();
            pyInstance = new CPyInstance();
            PyRun_SimpleString("import warnings;warnings.filterwarnings('ignore')");
        }
        return wrapper;
    }
    void PyWrap::RemoveInstance()
    {
        if (wrapper != nullptr) {
            if (pyInstance != nullptr) {
                delete pyInstance;
            }
            pyInstance = nullptr;
            if (wrapper != nullptr) {
                delete wrapper;
            }
            wrapper = nullptr;
        }
    }
    void PyWrap::importClass(const clfId_t id, const std::string& moduleName, const std::string& className)
    {
        // Validate input parameters for security
        validateModuleName(moduleName);
        validateClassName(className);
        
        std::lock_guard<std::mutex> lock(mutex);
        auto result = moduleClassMap.find(id);
        if (result != moduleClassMap.end()) {
            return;
        }
        
        // Acquire GIL for Python operations
        PyGILState_STATE gstate = PyGILState_Ensure();
        
        try {
            PyObject* module = PyImport_ImportModule(moduleName.c_str());
            if (PyErr_Occurred()) {
                PyGILState_Release(gstate);
                errorAbort("Couldn't import module " + moduleName);
            }
            
            PyObject* classObject = PyObject_GetAttrString(module, className.c_str());
            if (PyErr_Occurred()) {
                Py_DECREF(module);
                PyGILState_Release(gstate);
                errorAbort("Couldn't find class " + className);
            }
            
            PyObject* instance = PyObject_CallObject(classObject, NULL);
            if (PyErr_Occurred()) {
                Py_DECREF(module);
                Py_DECREF(classObject);
                PyGILState_Release(gstate);
                errorAbort("Couldn't create instance of class " + className);
            }
            
            moduleClassMap.insert({ id, { module, classObject, instance } });
            PyGILState_Release(gstate);
        }
        catch (...) {
            PyGILState_Release(gstate);
            throw;
        }
    }
    void PyWrap::clean(const clfId_t id)
    {
        // Remove Python interpreter if no more modules imported left
        std::lock_guard<std::mutex> lock(mutex);
        auto result = moduleClassMap.find(id);
        if (result == moduleClassMap.end()) {
            return;
        }
        Py_DECREF(std::get<0>(result->second));
        Py_DECREF(std::get<1>(result->second));
        Py_DECREF(std::get<2>(result->second));
        moduleClassMap.erase(result);
        if (PyErr_Occurred()) {
            PyErr_Print();
            errorAbort("Error cleaning module ");
        }
        // With boost you can't remove the interpreter
        // https://www.boost.org/doc/libs/1_83_0/libs/python/doc/html/tutorial/tutorial/embedding.html#tutorial.embedding.getting_started
        // if (moduleClassMap.empty()) {
        //     RemoveInstance();
        // }
    }
    void PyWrap::errorAbort(const std::string& message)
    {
        // Clear Python error state
        if (PyErr_Occurred()) {
            PyErr_Clear();
        }
        
        // Sanitize error message to prevent information disclosure
        std::string sanitizedMessage = sanitizeErrorMessage(message);
        
        // Throw exception instead of terminating process
        throw PyWrapException(sanitizedMessage);
    }
    
    void PyWrap::validateModuleName(const std::string& moduleName) {
        // Whitelist of allowed module names for security
        static const std::set<std::string> allowedModules = {
            "sklearn.svm", "sklearn.ensemble", "sklearn.tree",
            "xgboost", "numpy", "sklearn",
            "stree", "odte", "adaboost"
        };
        
        if (moduleName.empty()) {
            throw PyImportException("Module name cannot be empty");
        }
        
        // Check for path traversal attempts
        if (moduleName.find("..") != std::string::npos || 
            moduleName.find("/") != std::string::npos ||
            moduleName.find("\\") != std::string::npos) {
            throw PyImportException("Invalid characters in module name: " + moduleName);
        }
        
        // Check if module is in whitelist
        if (allowedModules.find(moduleName) == allowedModules.end()) {
            throw PyImportException("Module not in whitelist: " + moduleName);
        }
    }
    
    void PyWrap::validateClassName(const std::string& className) {
        if (className.empty()) {
            throw PyClassException("Class name cannot be empty");
        }
        
        // Check for dangerous characters
        if (className.find("__") != std::string::npos) {
            throw PyClassException("Invalid characters in class name: " + className);
        }
        
        // Must be valid Python identifier
        if (!std::isalpha(className[0]) && className[0] != '_') {
            throw PyClassException("Invalid class name format: " + className);
        }
        
        for (char c : className) {
            if (!std::isalnum(c) && c != '_') {
                throw PyClassException("Invalid character in class name: " + className);
            }
        }
    }
    
    void PyWrap::validateHyperparameters(const json& hyperparameters) {
        // Whitelist of allowed hyperparameter keys
        static const std::set<std::string> allowedKeys = {
            "random_state", "n_estimators", "max_depth", "learning_rate",
            "C", "gamma", "kernel", "degree", "coef0", "probability",
            "criterion", "splitter", "min_samples_split", "min_samples_leaf",
            "min_weight_fraction_leaf", "max_features", "max_leaf_nodes",
            "min_impurity_decrease", "bootstrap", "oob_score", "n_jobs",
            "verbose", "warm_start", "class_weight"
        };
        
        for (const auto& [key, value] : hyperparameters.items()) {
            if (allowedKeys.find(key) == allowedKeys.end()) {
                throw PyWrapException("Hyperparameter not in whitelist: " + key);
            }
            
            // Validate value types and ranges
            if (key == "random_state" && value.is_number_integer()) {
                int val = value.get<int>();
                if (val < 0 || val > 2147483647) {
                    throw PyWrapException("Invalid random_state value: " + std::to_string(val));
                }
            }
            else if (key == "n_estimators" && value.is_number_integer()) {
                int val = value.get<int>();
                if (val < 1 || val > 10000) {
                    throw PyWrapException("Invalid n_estimators value: " + std::to_string(val));
                }
            }
            else if (key == "max_depth" && value.is_number_integer()) {
                int val = value.get<int>();
                if (val < 1 || val > 1000) {
                    throw PyWrapException("Invalid max_depth value: " + std::to_string(val));
                }
            }
        }
    }
    
    std::string PyWrap::sanitizeErrorMessage(const std::string& message) {
        // Remove sensitive information from error messages
        std::string sanitized = message;
        
        // Remove file paths
        std::regex pathRegex(R"([A-Za-z]:[\\/.][^\s]+|/[^\s]+)");
        sanitized = std::regex_replace(sanitized, pathRegex, "[PATH_REMOVED]");
        
        // Remove memory addresses
        std::regex addrRegex(R"(0x[0-9a-fA-F]+)");
        sanitized = std::regex_replace(sanitized, addrRegex, "[ADDR_REMOVED]");
        
        // Limit message length
        if (sanitized.length() > 200) {
            sanitized = sanitized.substr(0, 200) + "...";
        }
        
        return sanitized;
    }
    PyObject* PyWrap::getClass(const clfId_t id)
    {
        std::lock_guard<std::mutex> lock(mutex); // Add thread safety
        auto item = moduleClassMap.find(id);
        if (item == moduleClassMap.end()) {
            throw std::runtime_error("Module not found for id: " + std::to_string(id));
        }
        return std::get<2>(item->second);
    }
    std::string PyWrap::callMethodString(const clfId_t id, const std::string& method)
    {
        // Acquire GIL for Python operations
        PyGILState_STATE gstate = PyGILState_Ensure();
        
        try {
            PyObject* instance = getClass(id);
            PyObject* result;
            
            if (!(result = PyObject_CallMethod(instance, method.c_str(), NULL))) {
                PyGILState_Release(gstate);
                errorAbort("Couldn't call method " + method);
            }
            
            std::string value = PyUnicode_AsUTF8(result);
            Py_XDECREF(result);
            PyGILState_Release(gstate);
            return value;
        }
        catch (const std::exception& e) {
            PyGILState_Release(gstate);
            errorAbort(e.what());
            return ""; // This line should never be reached due to errorAbort throwing
        }
        catch (...) {
            PyGILState_Release(gstate);
            throw;
        }
    }
    int PyWrap::callMethodInt(const clfId_t id, const std::string& method)
    {
        // Acquire GIL for Python operations
        PyGILState_STATE gstate = PyGILState_Ensure();
        
        try {
            PyObject* instance = getClass(id);
            PyObject* result;
            
            if (!(result = PyObject_CallMethod(instance, method.c_str(), NULL))) {
                PyGILState_Release(gstate);
                errorAbort("Couldn't call method " + method);
            }
            
            int value = PyLong_AsLong(result);
            Py_XDECREF(result);
            PyGILState_Release(gstate);
            return value;
        }
        catch (const std::exception& e) {
            PyGILState_Release(gstate);
            errorAbort(e.what());
            return 0; // This line should never be reached due to errorAbort throwing
        }
        catch (...) {
            PyGILState_Release(gstate);
            throw;
        }
    }
    std::string PyWrap::sklearnVersion()
    {
        // Acquire GIL for Python operations
        PyGILState_STATE gstate = PyGILState_Ensure();
        
        try {
            // Validate module name for security
            validateModuleName("sklearn");
            
            PyObject* sklearnModule = PyImport_ImportModule("sklearn");
            if (sklearnModule == nullptr) {
                PyGILState_Release(gstate);
                errorAbort("Couldn't import sklearn");
            }
            
            PyObject* versionAttr = PyObject_GetAttrString(sklearnModule, "__version__");
            if (versionAttr == nullptr || !PyUnicode_Check(versionAttr)) {
                Py_XDECREF(sklearnModule);
                PyGILState_Release(gstate);
                errorAbort("Couldn't get sklearn version");
            }
            
            std::string result = PyUnicode_AsUTF8(versionAttr);
            Py_XDECREF(versionAttr);
            Py_XDECREF(sklearnModule);
            PyGILState_Release(gstate);
            return result;
        }
        catch (...) {
            PyGILState_Release(gstate);
            throw;
        }
    }
    std::string PyWrap::version(const clfId_t id)
    {
        return callMethodString(id, "version");
    }
    int PyWrap::callMethodSumOfItems(const clfId_t id, const std::string& method)
    {
        // Acquire GIL for Python operations
        PyGILState_STATE gstate = PyGILState_Ensure();
        
        try {
            // Call method on each estimator and sum the results (made for RandomForest)
            PyObject* instance = getClass(id);
            PyObject* estimators = PyObject_GetAttrString(instance, "estimators_");
            if (estimators == nullptr) {
                PyGILState_Release(gstate);
                errorAbort("Failed to get attribute: " + method);
            }
            
            int sumOfItems = 0;
            Py_ssize_t len = PyList_Size(estimators);
            for (Py_ssize_t i = 0; i < len; i++) {
                PyObject* estimator = PyList_GetItem(estimators, i);
                PyObject* result;
                if (method == "node_count") {
                    PyObject* owner = PyObject_GetAttrString(estimator, "tree_");
                    if (owner == nullptr) {
                        Py_XDECREF(estimators);
                        PyGILState_Release(gstate);
                        errorAbort("Failed to get attribute tree_ for: " + method);
                    }
                    result = PyObject_GetAttrString(owner, method.c_str());
                    if (result == nullptr) {
                        Py_XDECREF(estimators);
                        Py_XDECREF(owner);
                        PyGILState_Release(gstate);
                        errorAbort("Failed to get attribute node_count: " + method);
                    }
                    Py_DECREF(owner);
                } else {
                    result = PyObject_CallMethod(estimator, method.c_str(), nullptr);
                    if (result == nullptr) {
                        Py_XDECREF(estimators);
                        PyGILState_Release(gstate);
                        errorAbort("Failed to call method: " + method);
                    }
                }
                sumOfItems += PyLong_AsLong(result);
                Py_DECREF(result);
            }
            Py_DECREF(estimators);
            PyGILState_Release(gstate);
            return sumOfItems;
        }
        catch (...) {
            PyGILState_Release(gstate);
            throw;
        }
    }
    void PyWrap::setHyperparameters(const clfId_t id, const json& hyperparameters)
    {
        // Validate hyperparameters for security
        validateHyperparameters(hyperparameters);
        
        // Acquire GIL for Python operations
        PyGILState_STATE gstate = PyGILState_Ensure();
        
        try {
            // Set hyperparameters as attributes of the class
            PyObject* pValue;
            PyObject* instance = getClass(id);
            
            for (const auto& [key, value] : hyperparameters.items()) {
                std::stringstream oss;
                oss << value.type_name();
                if (oss.str() == "string") {
                    pValue = Py_BuildValue("s", value.get<std::string>().c_str());
                } else {
                    if (value.is_number_integer()) {
                        pValue = Py_BuildValue("i", value.get<int>());
                    } else {
                        pValue = Py_BuildValue("f", value.get<double>());
                    }
                }
                
                if (!pValue) {
                    PyGILState_Release(gstate);
                    throw PyWrapException("Failed to create Python value for hyperparameter: " + key);
                }
                
                int res = PyObject_SetAttrString(instance, key.c_str(), pValue);
                if (res == -1 && PyErr_Occurred()) {
                    Py_XDECREF(pValue);
                    PyGILState_Release(gstate);
                    errorAbort("Couldn't set attribute " + key + "=" + value.dump());
                }
                Py_XDECREF(pValue);
            }
            PyGILState_Release(gstate);
        }
        catch (...) {
            PyGILState_Release(gstate);
            throw;
        }
    }
    void PyWrap::fit(const clfId_t id, CPyObject& X, CPyObject& y)
    {
        // Acquire GIL for Python operations
        PyGILState_STATE gstate = PyGILState_Ensure();
        
        try {
            PyObject* instance = getClass(id);
            CPyObject result;
            CPyObject method = PyUnicode_FromString("fit");
            
            if (!(result = PyObject_CallMethodObjArgs(instance, method.getObject(), X.getObject(), y.getObject(), NULL))) {
                PyGILState_Release(gstate);
                errorAbort("Couldn't call method fit");
            }
            PyGILState_Release(gstate);
        }
        catch (const std::exception& e) {
            PyGILState_Release(gstate);
            errorAbort(e.what());
        }
        catch (...) {
            PyGILState_Release(gstate);
            throw;
        }
    }
    PyObject* PyWrap::predict_proba(const clfId_t id, CPyObject& X)
    {
        return predict_method("predict_proba", id, X);
    }
    PyObject* PyWrap::predict(const clfId_t id, CPyObject& X)
    {
        return predict_method("predict", id, X);
    }
    PyObject* PyWrap::predict_method(const std::string name, const clfId_t id, CPyObject& X)
    {
        // Acquire GIL for Python operations
        PyGILState_STATE gstate = PyGILState_Ensure();
        
        try {
            PyObject* instance = getClass(id);
            PyObject* result;
            CPyObject method = PyUnicode_FromString(name.c_str());
            
            if (!(result = PyObject_CallMethodObjArgs(instance, method.getObject(), X.getObject(), NULL))) {
                PyGILState_Release(gstate);
                errorAbort("Couldn't call method " + name);
            }
            
            PyGILState_Release(gstate);
            // PyObject_CallMethodObjArgs already returns a new reference, no need for Py_INCREF
            return result; // Caller must free this object
        }
        catch (const std::exception& e) {
            PyGILState_Release(gstate);
            errorAbort(e.what());
            return nullptr; // This line should never be reached due to errorAbort throwing
        }
        catch (...) {
            PyGILState_Release(gstate);
            throw;
        }
    }
    double PyWrap::score(const clfId_t id, CPyObject& X, CPyObject& y)
    {
        // Acquire GIL for Python operations
        PyGILState_STATE gstate = PyGILState_Ensure();
        
        try {
            PyObject* instance = getClass(id);
            CPyObject result;
            CPyObject method = PyUnicode_FromString("score");
            
            if (!(result = PyObject_CallMethodObjArgs(instance, method.getObject(), X.getObject(), y.getObject(), NULL))) {
                PyGILState_Release(gstate);
                errorAbort("Couldn't call method score");
            }
            
            double resultValue = PyFloat_AsDouble(result);
            PyGILState_Release(gstate);
            return resultValue;
        }
        catch (const std::exception& e) {
            PyGILState_Release(gstate);
            errorAbort(e.what());
            return 0.0; // This line should never be reached due to errorAbort throwing
        }
        catch (...) {
            PyGILState_Release(gstate);
            throw;
        }
    }
}