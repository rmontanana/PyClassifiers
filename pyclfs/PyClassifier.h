#ifndef PYCLASSIFIER_H
#define PYCLASSIFIER_H
#include <string>
#include <map>
#include <vector>
#include <utility>
#include "boost/python/detail/wrap_python.hpp"
#include <boost/python/numpy.hpp>
#include <torch/torch.h>
#include <nlohmann/json.hpp>
#include "bayesnet/classifiers/Classifier.h"
#include "PyWrap.h"
#include "TypeId.h"

namespace pywrap {
    class PyClassifier : public bayesnet::BaseClassifier {
    public:
        PyClassifier(const std::string& module, const std::string& className, const bool sklearn = false);
        virtual ~PyClassifier();
        PyClassifier& fit(std::vector<std::vector<int>>& X, std::vector<int>& y, const std::vector<std::string>& features, const std::string& className, std::map<std::string, std::vector<int>>& states, const bayesnet::Smoothing_t smoothing = bayesnet::Smoothing_t::NONE) override { return *this; };
        // X is nxm tensor, y is nx1 tensor
        PyClassifier& fit(torch::Tensor& X, torch::Tensor& y, const std::vector<std::string>& features, const std::string& className, std::map<std::string, std::vector<int>>& states, const bayesnet::Smoothing_t smoothing = bayesnet::Smoothing_t::NONE) override;
        PyClassifier& fit(torch::Tensor& X, torch::Tensor& y);
        PyClassifier& fit(torch::Tensor& dataset, const std::vector<std::string>& features, const std::string& className, std::map<std::string, std::vector<int>>& states, const bayesnet::Smoothing_t smoothing = bayesnet::Smoothing_t::NONE) override { return *this; };
        PyClassifier& fit(torch::Tensor& dataset, const std::vector<std::string>& features, const std::string& className, std::map<std::string, std::vector<int>>& states, const torch::Tensor& weights, const bayesnet::Smoothing_t smoothing = bayesnet::Smoothing_t::NONE) override { return *this; };
        torch::Tensor predict(torch::Tensor& X) override;
        std::vector<int> predict(std::vector<std::vector<int >>& X) override { return std::vector<int>(); }; // Not implemented
        torch::Tensor predict_proba(torch::Tensor& X) override;
        std::vector<std::vector<double>> predict_proba(std::vector<std::vector<int >>& X) override { return std::vector<std::vector<double>>(); }; // Not implemented
        float score(std::vector<std::vector<int>>& X, std::vector<int>& y) override { return 0.0; }; // Not implemented
        float score(torch::Tensor& X, torch::Tensor& y) override;
        int getClassNumStates() const override { return 0; };
        std::string version();
        std::string callMethodString(const std::string& method);
        int callMethodSumOfItems(const std::string& method) const;
        int callMethodInt(const std::string& method) const;
        std::string getVersion() override { return this->version(); };
        int getNumberOfNodes() const override { return 0; };
        int getNumberOfEdges() const override { return 0; };
        int getNumberOfStates() const override { return 0; };
        std::vector<std::string> show() const override { return std::vector<std::string>(); }
        std::vector<std::string> graph(const std::string& title = "") const override { return std::vector<std::string>(); }
        bayesnet::status_t getStatus() const override { return bayesnet::NORMAL; };
        std::vector<std::string> topological_order() override { return std::vector<std::string>(); }
        std::string dump_cpt() const override { return ""; };
        std::vector<std::string> getNotes() const override { return notes; };
        void setHyperparameters(const nlohmann::json& hyperparameters) override;
    protected:
        nlohmann::json hyperparameters;
        void trainModel(const torch::Tensor& weights, const bayesnet::Smoothing_t smoothing = bayesnet::Smoothing_t::NONE) override {};
        std::vector<std::string> notes;
        bool xgboost = false;
    private:
        PyWrap* pyWrap;
        std::string module;
        std::string className;
        bool sklearn;
        clfId_t id;
        bool fitted;
    };
} /* namespace pywrap */
#endif /* PYCLASSIFIER_H */