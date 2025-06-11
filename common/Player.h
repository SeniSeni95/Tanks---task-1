#pragma once
#include "SatelliteView.h"
#include "TankAlgorithm.h"


class Player {
public:
	Player( [[maybe_unused]] int player_index,
        [[maybe_unused]] size_t x, [[maybe_unused]] size_t y,
        [[maybe_unused]] size_t max_steps, [[maybe_unused]] size_t num_shells ) {}
	virtual ~Player() {}
virtual void updateTankWithBattleInfo
(TankAlgorithm& tank, SatelliteView& satellite_view) = 0;
};
