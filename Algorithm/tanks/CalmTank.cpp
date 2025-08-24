#include "CalmTank.h"
#include <cstdlib>
#include <ctime>

unique_ptr<algorithm> CalmTank::createAlgorithm() {
    return make_unique<running_algorithm>();
}
