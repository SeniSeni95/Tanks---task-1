#pragma once

#include <iostream>  // for std::cout
#include "PlayerFactory.h"
#include "MyPlayer.h"

class MyPlayerFactory : public PlayerFactory {
public:
    std::unique_ptr<Player> create(int player_index, size_t x, size_t y,
                                   size_t max_steps, size_t num_shells) const override {
        // std::cout << "[DEBUG] MyPlayerFactory::create called with index=" << player_index
                //   << ", x=" << x << ", y=" << y
                //   << ", maxSteps=" << max_steps
                //   << ", numShells=" << num_shells << std::endl;

        auto player = std::make_unique<MyPlayer>(player_index, x, y, max_steps, num_shells);
        // std::cout << "[DEBUG] MyPlayerFactory::create returning player " << player_index << std::endl;
        return player;
    }
};
