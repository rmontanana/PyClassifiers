#ifndef ADABOOSTPY_H
#define ADABOOSTPY_H
#include "PyClassifier.h"

namespace pywrap {
    class AdaBoostPy : public PyClassifier {
    public:
        AdaBoostPy();
        ~AdaBoostPy() = default;
        int getNumberOfEdges() const override;
        int getNumberOfStates() const override;
        int getNumberOfNodes() const override;
    };
} /* namespace pywrap */
#endif /* ADABOOST_H */