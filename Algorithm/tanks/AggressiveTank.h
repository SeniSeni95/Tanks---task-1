#pragma once
#include "AbstractTankAlgorithm.h"
#include "MyBattleInfo.h"

namespace IDs_329506620_206055055 {

class AggressiveTank : public AbstractTankAlgorithm {
public:
    AggressiveTank(int player_index, int tank_index)
        : AbstractTankAlgorithm(player_index, tank_index) {}

    std::unique_ptr<algorithm> createAlgorithm() override;
};

} // namespace IDs_329506620_206055055
