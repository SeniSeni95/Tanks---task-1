#pragma once
#include "BattleInfo.h"
#include "ActionRequest.h"
#include <string>
#include <vector>
#include "Vector2D.h"
#include "BattleItem.h"
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

    void updateTankDirectionAndGear(int x, int y, int direction_x, int direction_y, const string &gear) {
        if (x < 0 || x >= board->n || y < 0 || y >= board->m) {
            throw out_of_range("Coordinates out of bounds");
        }

        cell &c = board->get_cell(x, y);
        for (const auto &obj : c.objects) {
            tank *t = dynamic_cast<tank *>(obj.get());
            if (t) {
                t->directionx = direction_x;
                t->directiony = direction_y;
                t->gear = gear;
                return;
            }
        }
        throw runtime_error("No tank found at the specified coordinates");
    }
};
