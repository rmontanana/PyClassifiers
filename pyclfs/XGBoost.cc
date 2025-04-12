#include "XGBoost.h"

//See https ://stackoverflow.com/questions/36071672/using-xgboost-in-c
namespace pywrap {
    XGBoost::XGBoost() : PyClassifier("xgboost", "XGBClassifier", true)
    {
        validHyperparameters = { "tree_method", "early_stopping_rounds", "n_jobs" };
        xgboost = true;
    }
} /* namespace pywrap */