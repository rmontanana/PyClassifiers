#include "STree.h"

namespace pywrap {
    STree::STree() : PyClassifier("stree", "Stree")
    {
        validHyperparameters = { "C", "kernel", "max_iter", "max_depth", "random_state", "multiclass_strategy", "gamma", "max_features", "degree" };
    };
    int STree::getNumberOfNodes() const
    {
        return callMethodInt("get_nodes");
    }
    int STree::getNumberOfEdges() const
    {
        return callMethodInt("get_leaves");
    }
    int STree::getNumberOfStates() const
    {
        return callMethodInt("get_depth");
    }
    std::string STree::graph()
    {
        return callMethodString("graph");
    }
} /* namespace pywrap */