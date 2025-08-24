#include "AggressiveTank.h"
#include <cstdlib>
#include <ctime>

unique_ptr<algorithm> AggressiveTank::createAlgorithm() {
    return make_unique<chasing_algorithm>();
}

