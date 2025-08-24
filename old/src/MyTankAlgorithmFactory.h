#pragma once

#include "MyTankAlgorithmFactory.h"
#include "MyTankAlgorithm.h"
#include <memory>

class MyTankAlgorithmFactory : public MyTankAlgorithmFactory {
public:
    std::unique_ptr<TankAlgorithm> create(int player_index, int tank_index) const override;
};
