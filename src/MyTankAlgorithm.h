#pragma once
#include "TankAlgorithm.h"
#include "ActionRequest.h"
#include "BattleInfo.h"
#include <string>

class MyTankAlgorithm : public TankAlgorithm {
private:
    BattleInfo* currentInfo = nullptr;

public:
    MyTankAlgorithm(int player_index, int tank_index);

    void updateBattleInfo(BattleInfo& info) override;

    ActionRequest getAction() override;
};
