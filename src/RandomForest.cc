#include "RandomForest.h"

namespace pywrap {
    RandomForest::RandomForest() : PyClassifier("sklearn.ensemble", "RandomForestClassifier", true)
    {
        validHyperparameters = { "n_estimators", "n_jobs", "random_state" };
    }
    int RandomForest::getNumberOfEdges() const
    {
        return callMethodSumOfItems("get_n_leaves");
    }
    int RandomForest::getNumberOfStates() const
    {
        return callMethodSumOfItems("get_depth");
    }
    int RandomForest::getNumberOfNodes() const
    {
        return callMethodSumOfItems("node_count");
    }
} /* namespace pywrap */