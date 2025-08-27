#pragma once
#include <functional>
#include <memory>

// Forward declaration for TankAlgorithm
class TankAlgorithm;

// Definition of MyTankAlgorithmFactory
using MyTankAlgorithmFactory = std::function<std::unique_ptr<TankAlgorithm>(int player_index, int tank_index)>;

struct TankAlgorithmRegistration {
  TankAlgorithmRegistration(MyTankAlgorithmFactory);
};

#define REGISTER_TANK_ALGORITHM(class_name) \
TankAlgorithmRegistration register_me_##class_name \
    ( [](int player_index, int tank_index) { \
        return std::make_unique<class_name>(player_index, tank_index); } \
    );
