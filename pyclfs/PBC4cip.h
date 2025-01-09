#ifndef PBC4CIP_H
#define PBC4CIP_H
#include "PyClassifier.h"

namespace pywrap {
    class PBC4cip : public PyClassifier {
    public:
        PBC4cip();
        ~PBC4cip() = default;
    };

} /* namespace pywrap */
#endif /* PBC4CIP_H */