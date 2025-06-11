#pragma once
#include "AbstractPlayer.h"

class CalmPlayer : public AbstractPlayer
{
public:
    CalmPlayer(int player_index, size_t x, size_t y,
                     size_t max_steps, size_t num_shells);
};