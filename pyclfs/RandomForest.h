#ifndef RANDOMFOREST_H
#define RANDOMFOREST_H
#include "PyClassifier.h"

namespace pywrap {
    class RandomForest : public PyClassifier {
    public:
        RandomForest();
        ~RandomForest() = default;
        int getNumberOfEdges() const override;
        int getNumberOfStates() const override;
        int getNumberOfNodes() const override;
    };
} /* namespace pywrap */
#endif /* RANDOMFOREST_H */