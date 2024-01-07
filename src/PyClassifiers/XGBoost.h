#ifndef XGBOOST_H
#define XGBOOST_H
#include "PyClassifier.h"

namespace pywrap {
    class XGBoost : public PyClassifier {
    public:
        XGBoost() : PyClassifier("xgboost", "XGBClassifier") {};
        ~XGBoost() = default;
        std::string version();
    };
} /* namespace pywrap */
#endif /* XGBOOST_H */