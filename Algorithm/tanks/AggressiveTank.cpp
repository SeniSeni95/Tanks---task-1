#include "AggressiveTank.h"
#include <cstdlib>
#include <ctime>

namespace IDs_329506620_206055055 {

std::unique_ptr<algorithm> AggressiveTank::createAlgorithm() {
    return std::make_unique<chasing_algorithm>();
}

} // namespace IDs_329506620_206055055

#include "../../Simulator/TankAlgorithmRegistration.h"

namespace {
    static TankAlgorithmRegistration register_me_aggressive_tank(
        [] (int player_index, int tank_index) {
            return std::make_unique<IDs_329506620_206055055::AggressiveTank>(
                player_index, tank_index
            );
        }
    );
}
