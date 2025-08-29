#pragma once
#include "AbstractPlayer.h"

namespace IDs_329506620_206055055 {

class AggressivePlayer : public AbstractPlayer
{
public:
    AggressivePlayer(int player_index, size_t x, size_t y,
                     size_t max_steps, size_t num_shells);
};

} // namespace IDs_329506620_206055055