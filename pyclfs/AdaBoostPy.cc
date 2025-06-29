#include "AdaBoostPy.h"

namespace pywrap {
    AdaBoostPy::AdaBoostPy() : PyClassifier("sklearn.ensemble", "AdaBoostClassifier", true)
    {
        validHyperparameters = { "n_estimators", "n_jobs", "random_state" };
    }
    int AdaBoostPy::getNumberOfEdges() const
    {
        return callMethodSumOfItems("get_n_leaves");
    }
    int AdaBoostPy::getNumberOfStates() const
    {
        return callMethodSumOfItems("get_depth");
    }
    int AdaBoostPy::getNumberOfNodes() const
    {
        return callMethodSumOfItems("node_count");
    }
} /* namespace pywrap */