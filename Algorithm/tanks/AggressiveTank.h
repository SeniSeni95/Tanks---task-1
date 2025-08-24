#pragma once
#include "AbstractTankAlgorithm.h"
#include "MyBattleInfo.h"

class AggressiveTank : public AbstractTankAlgorithm {
public:
    AggressiveTank(int player_index, int tank_index) : AbstractTankAlgorithm(player_index, tank_index) {}
    virtual unique_ptr<algorithm> createAlgorithm() override;
};
