#pragma once
#include <functional>
#include <memory>
#include "Player.h"

using PlayerFactory = std::function<std::unique_ptr<Player>(
    int player_index, size_t x, size_t y, size_t max_steps, size_t num_shells)>;
