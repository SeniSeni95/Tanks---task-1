#pragma once
#include "Player.h"
#include "TankAlgorithm.h"
#include "SatelliteView.h"

class MyPlayer : public Player {
public:
    MyPlayer(int player_index, size_t x, size_t y,
         size_t max_steps, size_t num_shells)
    : Player(player_index, x, y, max_steps, num_shells) {
    // std::cout << "[DEBUG] MyPlayer constructed" << std::endl;
}
    void updateTankWithBattleInfo(TankAlgorithm& /*tank*/, SatelliteView& /*satellite_view*/) override {
    std::cout << "a tank asked for battleinfo" << std::endl;
}
};

