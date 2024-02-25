#ifndef PYHELPER_HPP
#define PYHELPER_HPP
#pragma once
// Code taken and adapted from 
// https ://www.codeproject.com/Articles/820116/Embedding-Python-program-in-a-C-Cplusplus-code
#include "boost/python/detail/wrap_python.hpp"
#include <boost/python/numpy.hpp>
#include <iostream>

namespace pywrap {
    namespace p = boost::python;
    namespace np = boost::python::numpy;
    class CPyInstance {
    public:
        CPyInstance()
        {
            Py_Initialize();
            np::initialize();
        }

        ~CPyInstance()
        {
            Py_Finalize();
        }
    };
    class CPyObject {
    private:
        PyObject* p;
    public:
        CPyObject() : p(NULL)
        {
        }

        CPyObject(PyObject* _p) : p(_p)
        {
        }
        ~CPyObject()
        {
            Release();
        }
        PyObject* getObject()
        {
            return p;
        }
        PyObject* setObject(PyObject* _p)
        {
            return (p = _p);
        }
        PyObject* AddRef()
        {
            if (p) {
                Py_INCREF(p);
            }
            return p;
        }
        void Release()
        {
            if (p) {
                Py_XDECREF(p);
            }

            p = NULL;
        }
        PyObject* operator ->()
        {
            return p;
        }
        bool is()
        {
            return p ? true : false;
        }
        operator PyObject* ()
        {
            return p;
        }
        PyObject* operator = (PyObject* pp)
        {
            p = pp;
            return p;
        }
        operator bool()
        {
            return p ? true : false;
        }
    };
} /* namespace pywrap */
#endif