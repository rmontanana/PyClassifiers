#ifndef PYWRAP_H
#define PYWRAP_H
#include <string>
#include <map>
#include <tuple>
#include <mutex>
#include <regex>
#include <set>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include "boost/python/detail/wrap_python.hpp"
#include "PyHelper.hpp"
#include "TypeId.h"
#pragma once


namespace pywrap {
    /*
    Singleton class to handle Python/numpy interpreter.
    */
    using json = nlohmann::json;
    
    // Custom exception classes for PyWrap errors
    class PyWrapException : public std::runtime_error {
    public:
        explicit PyWrapException(const std::string& message) : std::runtime_error(message) {}
    };
    
    class PyImportException : public PyWrapException {
    public:
        explicit PyImportException(const std::string& module) 
            : PyWrapException("Failed to import Python module: " + module) {}
    };
    
    class PyClassException : public PyWrapException {
    public:
        explicit PyClassException(const std::string& className) 
            : PyWrapException("Failed to find Python class: " + className) {}
    };
    
    class PyInstanceException : public PyWrapException {
    public:
        explicit PyInstanceException(const std::string& className) 
            : PyWrapException("Failed to create instance of Python class: " + className) {}
    };
    
    class PyMethodException : public PyWrapException {
    public:
        explicit PyMethodException(const std::string& method) 
            : PyWrapException("Failed to call Python method: " + method) {}
    };
    class PyWrap {
    public:
        PyWrap() = default;
        PyWrap(PyWrap& other) = delete;
        static PyWrap* GetInstance();
        void operator=(const PyWrap&) = delete;
        ~PyWrap() = default;
        std::string callMethodString(const clfId_t id, const std::string& method);
        int callMethodInt(const clfId_t id, const std::string& method);
        std::string sklearnVersion();
        std::string version(const clfId_t id);
        int callMethodSumOfItems(const clfId_t id, const std::string& method);
        void setHyperparameters(const clfId_t id, const json& hyperparameters);
        void fit(const clfId_t id, CPyObject& X, CPyObject& y);
        PyObject* predict(const clfId_t id, CPyObject& X);
        PyObject* predict_proba(const clfId_t id, CPyObject& X);
        double score(const clfId_t id, CPyObject& X, CPyObject& y);
        void clean(const clfId_t id);
        void importClass(const clfId_t id, const std::string& moduleName, const std::string& className);
        PyObject* getClass(const clfId_t id);
    private:
        // Input validation and security
        void validateModuleName(const std::string& moduleName);
        void validateClassName(const std::string& className);
        void validateHyperparameters(const json& hyperparameters);
        std::string sanitizeErrorMessage(const std::string& message);
        // Only call RemoveInstance from clean method
        static void RemoveInstance();
        PyObject* predict_method(const std::string name, const clfId_t id, CPyObject& X);
        void errorAbort(const std::string& message);
        // No need to use static map here, since this class is a singleton
        std::map<clfId_t, std::tuple<PyObject*, PyObject*, PyObject*>> moduleClassMap;
        static CPyInstance* pyInstance;
        static PyWrap* wrapper;
        static std::mutex mutex;
    };
} /* namespace pywrap */
#endif /* PYWRAP_H */