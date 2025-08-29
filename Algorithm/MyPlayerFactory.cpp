#include "MyPlayerFactory.h"
#include "players/AggressivePlayer.h"

namespace IDs_329506620_206055055 {

std::unique_ptr<Player> MyPlayerFactory::create(int player_index, size_t x, size_t y,
                                                size_t max_steps, size_t num_shells) const {
    // always return AggressivePlayer now
    return std::make_unique<AggressivePlayer>(player_index, x, y, max_steps, num_shells);
}

} // namespace IDs_329506620_206055055
