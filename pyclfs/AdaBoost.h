#ifndef ADABOOST_H
#define ADABOOST_H
#include "PyClassifier.h"

namespace pywrap {
    class AdaBoost : public PyClassifier {
    public:
        AdaBoost();
        ~AdaBoost() = default;
        int getNumberOfEdges() const override;
        int getNumberOfStates() const override;
        int getNumberOfNodes() const override;
    };
} /* namespace pywrap */
#endif /* ADABOOST_H */