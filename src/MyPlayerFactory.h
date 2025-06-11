#pragma once

#include <iostream>  // for std::cout
#include "PlayerFactory.h"
#include "AggressivePlayer.h"
#include "CalmPlayer.h"

class MyPlayerFactory : public PlayerFactory {
public:
    std::unique_ptr<Player> create(int player_index, size_t x, size_t y,
                                   size_t max_steps, size_t num_shells) const override {
        if (player_index == 1) {
            return std::make_unique<AggressivePlayer>(player_index, x, y, max_steps, num_shells);
        } else {
            return std::make_unique<CalmPlayer>(player_index, x, y, max_steps, num_shells);
        }
    }
};
