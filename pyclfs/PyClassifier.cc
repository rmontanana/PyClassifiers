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
        int m = X.size(0);
        int n = X.size(1);
        auto Xn = np::from_data(X.data_ptr(), np::dtype::get_builtin<float>(), bp::make_tuple(m, n), bp::make_tuple(sizeof(X.dtype()) * 2 * n, sizeof(X.dtype()) * 2), bp::object());
        Xn = Xn.transpose();
        return Xn;
    }
    np::ndarray tensorInt2numpy(torch::Tensor& X)
    {
        int m = X.size(0);
        int n = X.size(1);
        auto Xn = np::from_data(X.data_ptr(), np::dtype::get_builtin<int>(), bp::make_tuple(m, n), bp::make_tuple(sizeof(X.dtype()) * 2 * n, sizeof(X.dtype()) * 2), bp::object());
        Xn = Xn.transpose();
        //std::cout << "Transposed array:\n" << boost::python::extract<char const*>(boost::python::str(Xn)) << std::endl;
        return Xn;
    }
    std::pair<np::ndarray, np::ndarray> tensors2numpy(torch::Tensor& X, torch::Tensor& y)
    {
        int n = X.size(1);
        auto yn = np::from_data(y.data_ptr(), np::dtype::get_builtin<int32_t>(), bp::make_tuple(n), bp::make_tuple(sizeof(y.dtype()) * 2), bp::object());
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
        auto [Xn, yn] = tensors2numpy(X, y);
        CPyObject Xp = bp::incref(bp::object(Xn).ptr());
        CPyObject yp = bp::incref(bp::object(yn).ptr());
        pyWrap->fit(id, Xp, yp);
        fitted = true;
        return *this;
    }
    PyClassifier& PyClassifier::fit(torch::Tensor& X, torch::Tensor& y, const std::vector<std::string>& features, const std::string& className, std::map<std::string, std::vector<int>>& states, const bayesnet::Smoothing_t smoothing)
    {
        return fit(X, y);
    }
    torch::Tensor PyClassifier::predict(torch::Tensor& X)
    {
        int dimension = X.size(1);
        CPyObject Xp;
        if (X.dtype() == torch::kInt32) {
            auto Xn = tensorInt2numpy(X);
            Xp = bp::incref(bp::object(Xn).ptr());
        } else {
            auto Xn = tensor2numpy(X);
            Xp = bp::incref(bp::object(Xn).ptr());
        }
        PyObject* incoming = pyWrap->predict(id, Xp);
        bp::handle<> handle(incoming);
        bp::object object(handle);
        np::ndarray prediction = np::from_object(object);
        if (PyErr_Occurred()) {
            PyErr_Print();
            throw std::runtime_error("Error creating object for predict in " + module + " and class " + className);
        }
        int* data = reinterpret_cast<int*>(prediction.get_data());
        std::vector<int> vPrediction(data, data + prediction.shape(0));
        auto resultTensor = torch::tensor(vPrediction, torch::kInt32);
        Py_XDECREF(incoming);
        return resultTensor;
    }
    torch::Tensor PyClassifier::predict_proba(torch::Tensor& X)
    {
        int dimension = X.size(1);
        CPyObject Xp;
        if (X.dtype() == torch::kInt32) {
            auto Xn = tensorInt2numpy(X);
            Xp = bp::incref(bp::object(Xn).ptr());
        } else {
            auto Xn = tensor2numpy(X);
            Xp = bp::incref(bp::object(Xn).ptr());
        }
        PyObject* incoming = pyWrap->predict_proba(id, Xp);
        bp::handle<> handle(incoming);
        bp::object object(handle);
        np::ndarray prediction = np::from_object(object);
        if (PyErr_Occurred()) {
            PyErr_Print();
            throw std::runtime_error("Error creating object for predict_proba in " + module + " and class " + className);
        }
        double* data = reinterpret_cast<double*>(prediction.get_data());
        std::vector<double> vPrediction(data, data + prediction.shape(0) * prediction.shape(1));
        auto resultTensor = torch::tensor(vPrediction, torch::kFloat64).reshape({ prediction.shape(0), prediction.shape(1) });
        Py_XDECREF(incoming);
        return resultTensor;
    }
    float PyClassifier::score(torch::Tensor& X, torch::Tensor& y)
    {
        auto [Xn, yn] = tensors2numpy(X, y);
        CPyObject Xp = bp::incref(bp::object(Xn).ptr());
        CPyObject yp = bp::incref(bp::object(yn).ptr());
        return pyWrap->score(id, Xp, yp);
    }
    void PyClassifier::setHyperparameters(const nlohmann::json& hyperparameters)
    {
        this->hyperparameters = hyperparameters;
    }
} /* namespace pywrap */