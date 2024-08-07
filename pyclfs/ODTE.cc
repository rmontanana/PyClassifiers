#include "ODTE.h"

namespace pywrap {
    ODTE::ODTE() : PyClassifier("odte", "Odte")
    {
        validHyperparameters = { "n_jobs", "n_estimators", "random_state", "max_samples", "max_features", "be_hyperparams" };
    }
    int ODTE::getNumberOfNodes() const
    {
        return callMethodInt("get_nodes");
    }
    int ODTE::getNumberOfEdges() const
    {
        return callMethodInt("get_leaves");
    }
    int ODTE::getNumberOfStates() const
    {
        return callMethodInt("get_depth");
    }
    std::string ODTE::graph()
    {
        return callMethodString("graph");
    }
} /* namespace pywrap */