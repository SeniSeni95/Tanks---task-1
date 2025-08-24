#pragma once
#ifndef TANK_ALGORITHM_FACTORY_H
#define TANK_ALGORITHM_FACTORY_H

#include <memory>              // FIX 1: for std::unique_ptr
#include "TankAlgorithm.h"     // FIX 2: for TankAlgorithm

class MyTankAlgorithmFactory {
public:
    virtual ~MyTankAlgorithmFactory() {}
    virtual std::unique_ptr<TankAlgorithm> create(
        int player_index, int tank_index) const = 0;
};

#endif
