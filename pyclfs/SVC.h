#ifndef SVC_H
#define SVC_H
#include "PyClassifier.h"

namespace pywrap {
    class SVC : public PyClassifier {
    public:
        SVC();
        ~SVC() = default;
    };

} /* namespace pywrap */
#endif /* SVC_H */