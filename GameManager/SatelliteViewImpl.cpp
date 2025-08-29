#include "SatelliteViewImpl.h"
#include <memory>


// Debug control - set to true to enable debugging, false to disable
bool DEBUG_SAT = true;

void SatelliteViewImpl::updateCopy(const game_board& original) {
    boardCopy = std::unique_ptr<game_board>(original.dummy_copy());
}

void SatelliteViewImpl::updateSymbols(const game_board& original) {
    boardCopy = original.symbol_copy();
    if (DEBUG_SAT) {
        std::cout << "[DEBUG] SatelliteView updated with symbol_copy()\n";
    }
}

char SatelliteViewImpl::getObjectAt(size_t x, size_t y) const {
    if (!boardCopy || x >= static_cast<size_t>(boardCopy->n) || y >= static_cast<size_t>(boardCopy->m)) {
        if (DEBUG_SAT) std::cout << "[DEBUG] SatView out of bounds (" << x << "," << y << ")\n";
        return '&';
    }

    const cell& c = boardCopy->get_cell(x, y);
    if (DEBUG_SAT) {
        std::cout << "[DEBUG] SatView (" << x << "," << y << ") objects=" << c.objects.size() << "\n";
    }

    if (c.objects.empty()) {
        return ' ';
    }

    for (const std::shared_ptr<game_object>& obj : c.objects) {
        char ch = obj->get_symbol();
        if (DEBUG_SAT) {
            std::cout << "[DEBUG]   -> obj.get_symbol() returned '" << ch << "'\n";
        }
        if (ch != ' ') return ch;
    }

    return ' ';
}

void SatelliteViewImpl::print_board() const {
    if (boardCopy) boardCopy->print_board();
}
