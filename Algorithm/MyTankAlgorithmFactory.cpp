#include <memory>
#include "TankAlgorithm.h"

class MyTankAlgorithmFactory {
public:
    std::unique_ptr<TankAlgorithm> create(int player_index, int tank_index) const;
};
