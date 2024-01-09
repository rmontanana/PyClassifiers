#include "XGBoost.h"




//See https ://stackoverflow.com/questions/36071672/using-xgboost-in-c






namespace pywrap {
    std::string XGBoost::version()
    {
        return callMethodString("1.0");
    }
} /* namespace pywrap */