#pragma once
#include "AbstractPlayer.h"

class AggressivePlayer : public AbstractPlayer
{
public:
    AggressivePlayer(int player_index, size_t x, size_t y,
                     size_t max_steps, size_t num_shells);
};