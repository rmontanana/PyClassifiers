#include "SVC.h"

namespace pywrap {
    SVC::SVC() : PyClassifier("sklearn.svm", "SVC", true)
    {
        validHyperparameters = { "C", "gamma", "kernel", "random_state" };
    }
} /* namespace pywrap */