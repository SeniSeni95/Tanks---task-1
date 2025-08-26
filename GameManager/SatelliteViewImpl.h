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
    const game_board* getBoardCopy() const {
        return boardCopy.get();
    }
    void updateCopy(const game_board& original);
    char getObjectAt(size_t x, size_t y) const override;
    void print_board() const;
};

#endif
