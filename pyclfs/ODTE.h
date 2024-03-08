#ifndef ODTE_H
#define ODTE_H
#include "nlohmann/json.hpp"
#include "PyClassifier.h"

namespace pywrap {
    class ODTE : public PyClassifier {
    public:
        ODTE();
        ~ODTE() = default;
        int getNumberOfNodes() const override;
        int getNumberOfEdges() const override;
        int getNumberOfStates() const override;
        std::string graph();
    };
} /* namespace pywrap */
#endif /* ODTE_H */