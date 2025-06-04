#pragma once
#ifndef TANK_ALGORITHM_H
#define TANK_ALGORITHM_H
#include "ActionRequest.h"
#include "BattleInfo.h"

class TankAlgorithm {
public:
	virtual ~TankAlgorithm() {}
virtual ActionRequest getAction() = 0;
virtual void updateBattleInfo(BattleInfo& info) = 0;
};

#endif