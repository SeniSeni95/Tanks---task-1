#ifndef SATELLITE_VIEW_IMPL_H
#define SATELLITE_VIEW_IMPL_H

#include "SatelliteView.h"
#include "Board.h"
#include <memory>
#include <cstddef>
#include <vector>
#include <tuple>
#include <string>
#include <unordered_set>

class SatelliteViewImpl : public SatelliteView {
private:
    std::unique_ptr<game_board> boardCopy;

public:
    void updateCopy(const game_board& original) {
        boardCopy = std::unique_ptr<game_board>(original.dummy_copy());
    }

    char getObjectAt(size_t x, size_t y) const override {
        if (!boardCopy || x >= static_cast<size_t>(boardCopy->n) || y >= static_cast<size_t>(boardCopy->m))
            return '&';

        const cell& c = boardCopy->arr[x][y];
        if (c.objects.empty()) return ' ';

        for (const std::shared_ptr<game_object>& obj : c.objects) {
            char ch = obj->get_symbol();
            if (ch != ' ') return ch;
        }

        return ' ';
    }

    void print_board() const {
        if (boardCopy) boardCopy->print_board();
    }

    std::unique_ptr<SatelliteViewImpl> simulate_step(
        const std::vector<std::tuple<int, int, int, int>>& shell_data,
        const std::vector<std::tuple<int, int, int, int, std::string>>& tank_data,
        const std::string& action
    ) const;
};

#endif