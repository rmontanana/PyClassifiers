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
    auto moduleClassMap = std::map<std::pair<std::string, std::string>, std::tuple<PyObject*, PyObject*, PyObject*>>();

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
        std::lock_guard<std::mutex> lock(mutex);
        auto result = moduleClassMap.find(id);
        if (result != moduleClassMap.end()) {
            return;
        }
        PyObject* module = PyImport_ImportModule(moduleName.c_str());
        if (PyErr_Occurred()) {
            errorAbort("Couldn't import module " + moduleName);
        }
        PyObject* classObject = PyObject_GetAttrString(module, className.c_str());
        if (PyErr_Occurred()) {
            errorAbort("Couldn't find class " + className);
        }
        PyObject* instance = PyObject_CallObject(classObject, NULL);
        if (PyErr_Occurred()) {
            errorAbort("Couldn't create instance of class " + className);
        }
        moduleClassMap.insert({ id, { module, classObject, instance } });
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
        std::cerr << message << std::endl;
        PyErr_Print();
        RemoveInstance();
        exit(1);
    }
    PyObject* PyWrap::getClass(const clfId_t id)
    {
        auto item = moduleClassMap.find(id);
        if (item == moduleClassMap.end()) {
            errorAbort("Module not found");
        }
        return std::get<2>(item->second);
    }
    std::string PyWrap::callMethodString(const clfId_t id, const std::string& method)
    {
        PyObject* instance = getClass(id);
        PyObject* result;
        try {
            if (!(result = PyObject_CallMethod(instance, method.c_str(), NULL)))
                errorAbort("Couldn't call method " + method);
        }
        catch (const std::exception& e) {
            errorAbort(e.what());
        }
        std::string value = PyUnicode_AsUTF8(result);
        Py_XDECREF(result);
        return value;
    }
    int PyWrap::callMethodInt(const clfId_t id, const std::string& method)
    {
        PyObject* instance = getClass(id);
        PyObject* result;
        try {
            if (!(result = PyObject_CallMethod(instance, method.c_str(), NULL)))
                errorAbort("Couldn't call method " + method);
        }
        catch (const std::exception& e) {
            errorAbort(e.what());
        }
        int value = PyLong_AsLong(result);
        Py_XDECREF(result);
        return value;
    }
    std::string PyWrap::sklearnVersion()
    {
        PyObject* sklearnModule = PyImport_ImportModule("sklearn");
        if (sklearnModule == nullptr) {
            errorAbort("Couldn't import sklearn");
        }
        PyObject* versionAttr = PyObject_GetAttrString(sklearnModule, "__version__");
        if (versionAttr == nullptr || !PyUnicode_Check(versionAttr)) {
            Py_XDECREF(sklearnModule);
            errorAbort("Couldn't get sklearn version");
        }
        std::string result = PyUnicode_AsUTF8(versionAttr);
        Py_XDECREF(versionAttr);
        Py_XDECREF(sklearnModule);
        return result;
    }
    std::string PyWrap::version(const clfId_t id)
    {
        return callMethodString(id, "version");
    }
    int PyWrap::callMethodSumOfItems(const clfId_t id, const std::string& method)
    {
        // Call method on each estimator and sum the results (made for RandomForest)
        PyObject* instance = getClass(id);
        PyObject* estimators = PyObject_GetAttrString(instance, "estimators_");
        if (estimators == nullptr) {
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
                    errorAbort("Failed to get attribute tree_ for: " + method);
                }
                result = PyObject_GetAttrString(owner, method.c_str());
                if (result == nullptr) {
                    Py_XDECREF(estimators);
                    Py_XDECREF(owner);
                    errorAbort("Failed to get attribute node_count: " + method);
                }
                Py_DECREF(owner);
            } else {
                result = PyObject_CallMethod(estimator, method.c_str(), nullptr);
                if (result == nullptr) {
                    Py_XDECREF(estimators);
                    errorAbort("Failed to call method: " + method);
                }
            }
            sumOfItems += PyLong_AsLong(result);
            Py_DECREF(result);
        }
        Py_DECREF(estimators);
        return sumOfItems;
    }
    void PyWrap::setHyperparameters(const clfId_t id, const json& hyperparameters)
    {
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
            int res = PyObject_SetAttrString(instance, key.c_str(), pValue);
            if (res == -1 && PyErr_Occurred()) {
                Py_XDECREF(pValue);
                errorAbort("Couldn't set attribute " + key + "=" + value.dump());
            }
            Py_XDECREF(pValue);
        }
    }
    void PyWrap::fit(const clfId_t id, CPyObject& X, CPyObject& y)
    {
        PyObject* instance = getClass(id);
        CPyObject result;
        CPyObject method = PyUnicode_FromString("fit");
        try {
            if (!(result = PyObject_CallMethodObjArgs(instance, method.getObject(), X.getObject(), y.getObject(), NULL)))
                errorAbort("Couldn't call method fit");
        }
        catch (const std::exception& e) {
            errorAbort(e.what());
        }
    }
    PyObject* PyWrap::predict(const clfId_t id, CPyObject& X)
    {
        PyObject* instance = getClass(id);
        PyObject* result;
        CPyObject method = PyUnicode_FromString("predict");
        try {
            if (!(result = PyObject_CallMethodObjArgs(instance, method.getObject(), X.getObject(), NULL)))
                errorAbort("Couldn't call method predict");
        }
        catch (const std::exception& e) {
            errorAbort(e.what());
        }
        Py_INCREF(result);
        return result; // Caller must free this object
    }
    double PyWrap::score(const clfId_t id, CPyObject& X, CPyObject& y)
    {
        PyObject* instance = getClass(id);
        CPyObject result;
        CPyObject method = PyUnicode_FromString("score");
        try {
            if (!(result = PyObject_CallMethodObjArgs(instance, method.getObject(), X.getObject(), y.getObject(), NULL)))
                errorAbort("Couldn't call method score");
        }
        catch (const std::exception& e) {
            errorAbort(e.what());
        }
        double resultValue = PyFloat_AsDouble(result);
        return resultValue;
    }
}