#include "AdaBoost.h"

namespace pywrap {
    AdaBoost::AdaBoost() : PyClassifier("sklearn.ensemble", "AdaBoostClassifier", true)
    {
        validHyperparameters = { "n_estimators", "n_jobs", "random_state" };
    }
    int AdaBoost::getNumberOfEdges() const
    {
        return callMethodSumOfItems("get_n_leaves");
    }
    int AdaBoost::getNumberOfStates() const
    {
        return callMethodSumOfItems("get_depth");
    }
    int AdaBoost::getNumberOfNodes() const
    {
        return callMethodSumOfItems("node_count");
    }
} /* namespace pywrap */