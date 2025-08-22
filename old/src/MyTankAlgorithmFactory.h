#pragma once

#include "TankAlgorithmFactory.h"
#include "MyTankAlgorithm.h"
#include <memory>

class MyTankAlgorithmFactory : public TankAlgorithmFactory {
public:
    std::unique_ptr<TankAlgorithm> create(int player_index, int tank_index) const override;
};
