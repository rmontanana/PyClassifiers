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
        CPyObject() : p(nullptr)
        {
        }

        CPyObject(PyObject* _p) : p(_p)
        {
        }

        // Copy constructor
        CPyObject(const CPyObject& other) : p(other.p)
        {
            if (p) {
                Py_INCREF(p);
            }
        }

        // Move constructor
        CPyObject(CPyObject&& other) noexcept : p(other.p)
        {
            other.p = nullptr;
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
            if (p != _p) {
                Release();  // Release old reference
                p = _p;
            }
            return p;
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
                p = nullptr;
            }
        }
        PyObject* operator ->()
        {
            return p;
        }
        bool is() const
        {
            return p != nullptr;
        }

        // Check if object is valid
        bool isValid() const
        {
            return p != nullptr;
        }
        operator PyObject* ()
        {
            return p;
        }
        // Copy assignment operator
        CPyObject& operator=(const CPyObject& other)
        {
            if (this != &other) {
                Release();  // Release current reference
                p = other.p;
                if (p) {
                    Py_INCREF(p);  // Add reference to new object
                }
            }
            return *this;
        }

        // Move assignment operator
        CPyObject& operator=(CPyObject&& other) noexcept
        {
            if (this != &other) {
                Release();  // Release current reference
                p = other.p;
                other.p = nullptr;
            }
            return *this;
        }

        // Assignment from PyObject* - DEPRECATED, use setObject() instead
        PyObject* operator=(PyObject* pp)
        {
            setObject(pp);
            return p;
        }
        explicit operator bool() const
        {
            return p != nullptr;
        }
    };

    // RAII guard for PyObject* - safer alternative to manual reference management
    class PyObjectGuard {
    private:
        PyObject* obj_;
        bool owns_reference_;

    public:
        // Constructor takes ownership of a new reference
        explicit PyObjectGuard(PyObject* obj = nullptr) : obj_(obj), owns_reference_(true) {}
        
        // Constructor for borrowed references
        PyObjectGuard(PyObject* obj, bool borrow) : obj_(obj), owns_reference_(!borrow) {
            if (borrow && obj_) {
                Py_INCREF(obj_);
                owns_reference_ = true;
            }
        }

        // Non-copyable to prevent accidental reference issues
        PyObjectGuard(const PyObjectGuard&) = delete;
        PyObjectGuard& operator=(const PyObjectGuard&) = delete;

        // Movable
        PyObjectGuard(PyObjectGuard&& other) noexcept 
            : obj_(other.obj_), owns_reference_(other.owns_reference_) {
            other.obj_ = nullptr;
            other.owns_reference_ = false;
        }

        PyObjectGuard& operator=(PyObjectGuard&& other) noexcept {
            if (this != &other) {
                reset();
                obj_ = other.obj_;
                owns_reference_ = other.owns_reference_;
                other.obj_ = nullptr;
                other.owns_reference_ = false;
            }
            return *this;
        }

        ~PyObjectGuard() {
            reset();
        }

        // Reset to nullptr, releasing current reference if owned
        void reset(PyObject* new_obj = nullptr) {
            if (owns_reference_ && obj_) {
                Py_DECREF(obj_);
            }
            obj_ = new_obj;
            owns_reference_ = (new_obj != nullptr);
        }

        // Release ownership and return the object
        PyObject* release() {
            PyObject* result = obj_;
            obj_ = nullptr;
            owns_reference_ = false;
            return result;
        }

        // Get the raw pointer (does not transfer ownership)
        PyObject* get() const {
            return obj_;
        }

        // Check if valid
        bool isValid() const {
            return obj_ != nullptr;
        }

        explicit operator bool() const {
            return obj_ != nullptr;
        }

        // Access operators
        PyObject* operator->() const {
            return obj_;
        }

        // Implicit conversion to PyObject* for API calls (does not transfer ownership)
        operator PyObject*() const {
            return obj_;
        }
    };

    // Helper function to create a PyObjectGuard from a borrowed reference
    inline PyObjectGuard borrowReference(PyObject* obj) {
        return PyObjectGuard(obj, true);
    }

    // Helper function to create a PyObjectGuard from a new reference  
    inline PyObjectGuard newReference(PyObject* obj) {
        return PyObjectGuard(obj);
    }
} /* namespace pywrap */
#endif