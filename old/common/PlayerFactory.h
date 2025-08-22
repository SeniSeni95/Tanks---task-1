#pragma once

#include "Player.h"
#include <memory>             // ✅ Add this at the top
using std::unique_ptr;  

class PlayerFactory {
public:
	virtual ~PlayerFactory() {}
virtual unique_ptr<Player> create(int player_index, size_t x, size_t y,
                size_t max_steps, size_t num_shells ) const = 0;
};
