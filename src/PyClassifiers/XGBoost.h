#ifndef XGBOOST_H
#define XGBOOST_H
#include "PyClassifier.h"

namespace pywrap {
    class XGBoost : public PyClassifier {
    public:
        XGBoost();
        ~XGBoost() = default;
    };
} /* namespace pywrap */
#endif /* XGBOOST_H */