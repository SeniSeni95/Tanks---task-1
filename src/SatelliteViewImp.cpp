#include "SatelliteViewImpl.h"
#include <memory>

void SatelliteViewImpl::updateCopy(const game_board& original) {
    boardCopy = std::unique_ptr<game_board>(original.dummy_copy());
}

char SatelliteViewImpl::getObjectAt(size_t x, size_t y) const {
    if (!boardCopy || x >= static_cast<size_t>(boardCopy->n) || y >= static_cast<size_t>(boardCopy->m))
        return '&';

    const cell& c = boardCopy->get_cell(x, y);
    if (c.objects.empty()) return ' ';

    for (const std::shared_ptr<game_object>& obj : c.objects) {
        char ch = obj->get_symbol();
        if (ch != ' ') return ch;
    }

    return ' ';
}

void SatelliteViewImpl::print_board() const {
    if (boardCopy) boardCopy->print_board();
}
