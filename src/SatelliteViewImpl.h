#ifndef SATELLITE_VIEW_IMPL_H
#define SATELLITE_VIEW_IMPL_H

#include "SatelliteView.h"
#include "Board.h"
#include <memory>
#include <cstddef>

class SatelliteViewImpl : public SatelliteView {
private:
    std::unique_ptr<game_board> boardCopy;

public:
    void updateCopy(const game_board& original) {
        boardCopy = std::unique_ptr<game_board>(original.deep_copy());
    }

    char getObjectAt(size_t x, size_t y) const override {
        if (!boardCopy || x >= static_cast<size_t>(boardCopy->n) || y >= static_cast<size_t>(boardCopy->m))
            return '&';

        const cell& c = boardCopy->arr[x][y];
        if (c.objects.empty()) return ' ';

        for (const std::shared_ptr<game_object>& obj : c.objects) {
            char ch = obj->symbol;
            if (ch != ' ') return ch;
        }

        return ' ';
    }
};

#endif // SATELLITE_VIEW_IMPL_H
