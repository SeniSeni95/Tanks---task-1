#pragma once
#include <functional>
#include <memory>
#include "TankAlgorithm.h"

using MyTankAlgorithmFactory = std::function<std::unique_ptr<TankAlgorithm>(
    int player_index, int tank_index)>;
