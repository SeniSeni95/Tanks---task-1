#pragma once
#include "AbstractTankAlgorithm.h"
#include "MyBattleInfo.h"

class AggressiveTank : public AbstractTankAlgorithm {
public:
    virtual unique_ptr<algorithm> createAlgorithm() override;
};
