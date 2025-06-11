#pragma once
#include "TankAlgorithm.h"
#include "ActionRequest.h"
#include "BattleInfo.h"
#include <string>

class MyTankAlgorithm : public TankAlgorithm {
private:
    int player_index;
    int tank_index;
    BattleInfo* currentInfo = nullptr;
public:
    MyTankAlgorithm(int player_index, int tank_index);
    // int get_player_index() const override;   // <-- Fix: const and override
    // int get_tank_index() const override;     // <-- Fix: const and override
    void updateBattleInfo(BattleInfo& info) override;
    ActionRequest getAction() override;
};