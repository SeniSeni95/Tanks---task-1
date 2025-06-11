#pragma once
#include "AbstractTankAlgorithm.h"

using namespace std;

class CalmTank : public AbstractTankAlgorithm {
public:
    CalmTank(int player_index, int tank_index) : AbstractTankAlgorithm(player_index, tank_index) {}
    virtual unique_ptr<algorithm> createAlgorithm() override;
};
