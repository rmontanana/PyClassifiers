#ifndef PYWRAP_H
#define PYWRAP_H
#include "boost/python/detail/wrap_python.hpp"
#include <string>
#include <map>
#include <tuple>
#include <mutex>
#include <nlohmann/json.hpp>
#include "PyHelper.hpp"
#include "TypeId.h"
#pragma once


namespace pywrap {
    /*
    Singleton class to handle Python/numpy interpreter.
    */
    using json = nlohmann::json;
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
        double score(const clfId_t id, CPyObject& X, CPyObject& y);
        void clean(const clfId_t id);
        void importClass(const clfId_t id, const std::string& moduleName, const std::string& className);
        PyObject* getClass(const clfId_t id);
    private:
        // Only call RemoveInstance from clean method
        static void RemoveInstance();
        void errorAbort(const std::string& message);
        // No need to use static map here, since this class is a singleton
        std::map<clfId_t, std::tuple<PyObject*, PyObject*, PyObject*>> moduleClassMap;
        static CPyInstance* pyInstance;
        static PyWrap* wrapper;
        static std::mutex mutex;
    };
} /* namespace pywrap */
#endif /* PYWRAP_H */