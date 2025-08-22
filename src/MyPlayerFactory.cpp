#include "MyPlayerFactory.h"
#include "players/AggressivePlayer.h"
#include "players/CalmPlayer.h"

std::unique_ptr<Player> MyPlayerFactory::create(int player_index, size_t x, size_t y,
                                                size_t max_steps, size_t num_shells) const {
    if (player_index == 1) {
        return std::make_unique<AggressivePlayer>(player_index, x, y, max_steps, num_shells);
    } else {
        return std::make_unique<CalmPlayer>(player_index, x, y, max_steps, num_shells);
    }
}
