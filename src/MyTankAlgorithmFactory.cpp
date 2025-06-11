#include "MyTankAlgorithmFactory.h"
#include "TankAlgorithmFactory.h"
#include "tanks/AggressiveTank.h"
#include "tanks/CalmTank.h"


std::unique_ptr<TankAlgorithm> MyTankAlgorithmFactory::create(int player_index, int tank_index) const {
    unique_ptr<AbstractTankAlgorithm> algo;
    
    if (player_index == 1) {
        algo = std::make_unique<AggressiveTank>(player_index, tank_index);
    } else {
        algo = std::make_unique<CalmTank>(player_index, tank_index);
    }

    algo->initialize(); // Initialize the algorithm
    
    return std::move(algo);
}
