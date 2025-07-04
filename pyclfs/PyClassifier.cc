#include "PyClassifier.h"
namespace pywrap {
    namespace bp = boost::python;
    namespace np = boost::python::numpy;
    PyClassifier::PyClassifier(const std::string& module, const std::string& className, bool sklearn) : module(module), className(className), sklearn(sklearn), fitted(false)
    {
        // This id allows to have more than one instance of the same module/class
        id = reinterpret_cast<clfId_t>(this);
        pyWrap = PyWrap::GetInstance();
        pyWrap->importClass(id, module, className);
    }
    PyClassifier::~PyClassifier()
    {
        pyWrap->clean(id);
    }
    np::ndarray tensor2numpy(torch::Tensor& X)
    {
        // Validate tensor dimensions
        if (X.dim() != 2) {
            throw std::runtime_error("tensor2numpy: Expected 2D tensor, got " + std::to_string(X.dim()) + "D");
        }
        
        // Ensure tensor is contiguous and in the expected format
        auto X_copy = X.contiguous();
        
        if (X_copy.dtype() != torch::kFloat32) {
            throw std::runtime_error("tensor2numpy: Expected float32 tensor");
        }
        
        // Transpose from [features, samples] to [samples, features] for Python classifiers
        X_copy = X_copy.transpose(0, 1);
        
        int64_t m = X_copy.size(0);
        int64_t n = X_copy.size(1);
        
        // Calculate correct strides in bytes
        int64_t element_size = X_copy.element_size();
        int64_t stride0 = X_copy.stride(0) * element_size;
        int64_t stride1 = X_copy.stride(1) * element_size;
        
        auto Xn = np::from_data(X_copy.data_ptr(), np::dtype::get_builtin<float>(), 
                               bp::make_tuple(m, n), 
                               bp::make_tuple(stride0, stride1), 
                               bp::object());
        return Xn;
    }
    np::ndarray tensorInt2numpy(torch::Tensor& X)
    {
        // Validate tensor dimensions
        if (X.dim() != 2) {
            throw std::runtime_error("tensorInt2numpy: Expected 2D tensor, got " + std::to_string(X.dim()) + "D");
        }
        
        // Ensure tensor is contiguous and in the expected format
        auto X_copy = X.contiguous();
        
        if (X_copy.dtype() != torch::kInt32) {
            throw std::runtime_error("tensorInt2numpy: Expected int32 tensor");
        }
        
        // Transpose from [features, samples] to [samples, features] for Python classifiers
        X_copy = X_copy.transpose(0, 1);
        
        int64_t m = X_copy.size(0);
        int64_t n = X_copy.size(1);
        
        // Calculate correct strides in bytes
        int64_t element_size = X_copy.element_size();
        int64_t stride0 = X_copy.stride(0) * element_size;
        int64_t stride1 = X_copy.stride(1) * element_size;
        
        auto Xn = np::from_data(X_copy.data_ptr(), np::dtype::get_builtin<int>(), 
                               bp::make_tuple(m, n), 
                               bp::make_tuple(stride0, stride1), 
                               bp::object());
        return Xn;
    }
    std::pair<np::ndarray, np::ndarray> tensors2numpy(torch::Tensor& X, torch::Tensor& y)
    {
        // Validate y tensor dimensions
        if (y.dim() != 1) {
            throw std::runtime_error("tensors2numpy: Expected 1D y tensor, got " + std::to_string(y.dim()) + "D");
        }
        
        // Validate dimensions match (X is [features, samples], y is [samples])
        // X.size(1) is samples, y.size(0) is samples
        if (X.size(1) != y.size(0)) {
            throw std::runtime_error("tensors2numpy: X and y dimension mismatch: X[" + 
                                   std::to_string(X.size(1)) + "], y[" + std::to_string(y.size(0)) + "]");
        }
        
        // Ensure y tensor is contiguous
        y = y.contiguous();
        
        if (y.dtype() != torch::kInt32) {
            throw std::runtime_error("tensors2numpy: Expected int32 y tensor");
        }
        
        int64_t n = y.size(0);
        int64_t element_size = y.element_size();
        int64_t stride = y.stride(0) * element_size;
        
        auto yn = np::from_data(y.data_ptr(), np::dtype::get_builtin<int32_t>(), 
                               bp::make_tuple(n), 
                               bp::make_tuple(stride), 
                               bp::object());
        
        if (X.dtype() == torch::kInt32) {
            return { tensorInt2numpy(X), yn };
        }
        return { tensor2numpy(X), yn };
    }
    std::string PyClassifier::version()
    {
        if (sklearn) {
            return pyWrap->sklearnVersion();
        }
        return pyWrap->version(id);
    }
    std::string PyClassifier::callMethodString(const std::string& method)
    {
        return pyWrap->callMethodString(id, method);
    }
    int PyClassifier::callMethodSumOfItems(const std::string& method) const
    {
        return pyWrap->callMethodSumOfItems(id, method);
    }
    int PyClassifier::callMethodInt(const std::string& method) const
    {
        return pyWrap->callMethodInt(id, method);
    }
    PyClassifier& PyClassifier::fit(torch::Tensor& X, torch::Tensor& y)
    {
        if (!fitted && hyperparameters.size() > 0) {
            pyWrap->setHyperparameters(id, hyperparameters);
        }
        try {
            auto [Xn, yn] = tensors2numpy(X, y);
            CPyObject Xp = bp::incref(bp::object(Xn).ptr());
            CPyObject yp = bp::incref(bp::object(yn).ptr());
            pyWrap->fit(id, Xp, yp);
            fitted = true;
            return *this;
        }
        catch (const std::exception& e) {
            // Clear any Python errors before re-throwing
            if (PyErr_Occurred()) {
                PyErr_Clear();
            }
            throw;
        }
    }
    PyClassifier& PyClassifier::fit(torch::Tensor& X, torch::Tensor& y, const std::vector<std::string>& features, const std::string& className, std::map<std::string, std::vector<int>>& states, const bayesnet::Smoothing_t smoothing)
    {
        return fit(X, y);
    }
    torch::Tensor PyClassifier::predict(torch::Tensor& X)
    {
        try {
            CPyObject Xp;
            if (X.dtype() == torch::kInt32) {
                auto Xn = tensorInt2numpy(X);
                Xp = bp::incref(bp::object(Xn).ptr());
            } else {
                auto Xn = tensor2numpy(X);
                Xp = bp::incref(bp::object(Xn).ptr());
            }
            
            // Use RAII guard for automatic cleanup
            PyObjectGuard incoming(pyWrap->predict(id, Xp));
            if (!incoming) {
                throw std::runtime_error("predict() returned NULL for " + module + ":" + className);
            }
            
            bp::handle<> handle(incoming.release());  // Transfer ownership to boost
            bp::object object(handle);
            np::ndarray prediction = np::from_object(object);
            
            if (PyErr_Occurred()) {
                PyErr_Clear();
                throw std::runtime_error("Error creating numpy object for predict in " + module + ":" + className);
            }
            
            // Validate numpy array
            if (prediction.get_nd() != 1) {
                throw std::runtime_error("Expected 1D prediction array, got " + std::to_string(prediction.get_nd()) + "D");
            }
            
            // Safe type conversion with validation
            std::vector<int> vPrediction;
            if (xgboost) {
                // Validate data type for XGBoost (typically returns long)
                if (prediction.get_dtype() == np::dtype::get_builtin<long>()) {
                    long* data = reinterpret_cast<long*>(prediction.get_data());
                    vPrediction.reserve(prediction.shape(0));
                    for (int i = 0; i < prediction.shape(0); ++i) {
                        vPrediction.push_back(static_cast<int>(data[i]));
                    }
                } else {
                    throw std::runtime_error("XGBoost prediction: unexpected data type");
                }
            } else {
                // Validate data type for other classifiers (typically returns int)
                if (prediction.get_dtype() == np::dtype::get_builtin<int>()) {
                    int* data = reinterpret_cast<int*>(prediction.get_data());
                    vPrediction.assign(data, data + prediction.shape(0));
                } else {
                    throw std::runtime_error("Prediction: unexpected data type");
                }
            }
            
            return torch::tensor(vPrediction, torch::kInt32);
        }
        catch (const std::exception& e) {
            // Clear any Python errors before re-throwing
            if (PyErr_Occurred()) {
                PyErr_Clear();
            }
            throw;
        }
    }
    torch::Tensor PyClassifier::predict_proba(torch::Tensor& X)
    {
        try {
            CPyObject Xp;
            if (X.dtype() == torch::kInt32) {
                auto Xn = tensorInt2numpy(X);
                Xp = bp::incref(bp::object(Xn).ptr());
            } else {
                auto Xn = tensor2numpy(X);
                Xp = bp::incref(bp::object(Xn).ptr());
            }
            
            // Use RAII guard for automatic cleanup
            PyObjectGuard incoming(pyWrap->predict_proba(id, Xp));
            if (!incoming) {
                throw std::runtime_error("predict_proba() returned NULL for " + module + ":" + className);
            }
            
            bp::handle<> handle(incoming.release());  // Transfer ownership to boost
            bp::object object(handle);
            np::ndarray prediction = np::from_object(object);
            
            if (PyErr_Occurred()) {
                PyErr_Clear();
                throw std::runtime_error("Error creating numpy object for predict_proba in " + module + ":" + className);
            }
            
            // Validate numpy array dimensions
            if (prediction.get_nd() != 2) {
                throw std::runtime_error("Expected 2D probability array, got " + std::to_string(prediction.get_nd()) + "D");
            }
            
            int64_t rows = prediction.shape(0);
            int64_t cols = prediction.shape(1);
            
            // Safe type conversion with validation
            if (xgboost) {
                // Validate data type for XGBoost (typically returns float)
                if (prediction.get_dtype() == np::dtype::get_builtin<float>()) {
                    float* data = reinterpret_cast<float*>(prediction.get_data());
                    std::vector<float> vPrediction(data, data + rows * cols);
                    return torch::tensor(vPrediction, torch::kFloat32).reshape({rows, cols});
                } else {
                    throw std::runtime_error("XGBoost predict_proba: unexpected data type");
                }
            } else {
                // Validate data type for other classifiers (typically returns double)
                if (prediction.get_dtype() == np::dtype::get_builtin<double>()) {
                    double* data = reinterpret_cast<double*>(prediction.get_data());
                    std::vector<double> vPrediction(data, data + rows * cols);
                    return torch::tensor(vPrediction, torch::kFloat64).reshape({rows, cols});
                } else {
                    throw std::runtime_error("predict_proba: unexpected data type");
                }
            }
        }
        catch (const std::exception& e) {
            // Clear any Python errors before re-throwing
            if (PyErr_Occurred()) {
                PyErr_Clear();
            }
            throw;
        }
    }
    float PyClassifier::score(torch::Tensor& X, torch::Tensor& y)
    {
        try {
            auto [Xn, yn] = tensors2numpy(X, y);
            CPyObject Xp = bp::incref(bp::object(Xn).ptr());
            CPyObject yp = bp::incref(bp::object(yn).ptr());
            return pyWrap->score(id, Xp, yp);
        }
        catch (const std::exception& e) {
            // Clear any Python errors before re-throwing
            if (PyErr_Occurred()) {
                PyErr_Clear();
            }
            throw;
        }
    }
    void PyClassifier::setHyperparameters(const nlohmann::json& hyperparameters)
    {
        this->hyperparameters = hyperparameters;
    }
} /* namespace pywrap */
