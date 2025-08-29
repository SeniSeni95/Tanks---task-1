#include "AggressivePlayer.h"

namespace IDs_329506620_206055055 {

AggressivePlayer::AggressivePlayer(int player_index, size_t x, size_t y,
                                   size_t max_steps, size_t num_shells)
    : AbstractPlayer(player_index, x, y, max_steps, num_shells) {}

} // namespace IDs_329506620_206055055

#include "../../Simulator/PlayerRegistration.h"

namespace {
    static PlayerRegistration register_me_aggressive(
        [] (int idx, size_t x, size_t y, size_t steps, size_t shells) {
            return std::make_unique<IDs_329506620_206055055::AggressivePlayer>(
                idx, x, y, steps, shells
            );
        }
    );
}
