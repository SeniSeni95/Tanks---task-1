
#include "TankAlgorithmFactory.h"
#include "tanks/AggressiveTank.h"
#include "tanks/CalmTank.h"

class MyTankAlgorithmFactory : public TankAlgorithmFactory {
public:
    std::unique_ptr<TankAlgorithm> create(int player_index, int tank_index) const override {
        if (player_index == 1) {
            return std::make_unique<AggressiveTank>(player_index, tank_index);
        } else {
            return std::make_unique<CalmTank>(player_index, tank_index);
        }
    }
};
