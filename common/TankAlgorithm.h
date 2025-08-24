#pragma once
#ifndef TANK_ALGORITHM_H
#define TANK_ALGORITHM_H
#include "ActionRequest.h"
#include "../common/TankAlgorithm.h"
#include "BattleInfo.h"
#include <functional>
#include <memory>

class TankAlgorithm {
public:
	virtual ~TankAlgorithm() {}
    virtual ActionRequest getAction() = 0;
    virtual void updateBattleInfo(BattleInfo& info) = 0;
};

using TankAlgorithmFactory =
std::function<std::unique_ptr<TankAlgorithm>(int player_index, int tank_index)>;


#endif

