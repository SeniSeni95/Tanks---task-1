#pragma once
#include "BattleInfo.h"
#include "ActionRequest.h"
#include <string>
#include <vector>
#include "Vector2D.h"
#include "Board.h"
#include "SatelliteViewImpl.h"

using namespace std;

class MyBattleInfo : public BattleInfo
{
private:
    tuple<int, int, int, int, string> selfTank; // x, y, direction_x, direction_y, gear
    unique_ptr<game_board> board;

public:
    MyBattleInfo(unique_ptr<game_board> board) : board(std::move(board)) {}
    virtual ~MyBattleInfo() = default;

    game_board *getBoard() const {
        return board.get();
    }

    tuple<int, int, int, int, string> getSelfTank() const {
        return selfTank;
    }

    void setSelfTank(tuple<int, int, int, int, string> t) {
        selfTank = t;
    }
};
