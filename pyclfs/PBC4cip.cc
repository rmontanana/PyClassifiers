#include "PBC4cip.h"

namespace pywrap {
    PBC4cip::PBC4cip() : PyClassifier("core.PBC4cip", "PBC4cip", true)
    {
        validHyperparameters = { "random_state" };
    }
} /* namespace pywrap */