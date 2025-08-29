#pragma once

#include <memory>
#include "../common/PlayerFactory.h"

namespace IDs_329506620_206055055 {

class MyPlayerFactory {
public:
    std::unique_ptr<Player> create(int player_index, size_t x, size_t y,
                                   size_t max_steps, size_t num_shells) const;
};

} // namespace IDs_329506620_206055055
