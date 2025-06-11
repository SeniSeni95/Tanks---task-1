#include "SatelliteViewImpl.h"
#include "GameObject.h"

std::unique_ptr<game_board> SatelliteViewImpl::generate_board(
    const game_board& base_board,
    const std::vector<std::tuple<int, int, int, int>>& shell_data,
    const std::vector<std::tuple<int, int, int, int, std::string>>& tank_data
) {
    auto new_board = std::unique_ptr<game_board>(base_board.dummy_copy());

    // Place/update shells
    for (const auto& shell_info : shell_data) {
        int x, y, dirx, diry;
        std::tie(x, y, dirx, diry) = shell_info;
        bool found = false;
        for (auto& s_ptr : new_board->shells) {
            shell* s = s_ptr.get();
            if (s->get_x() == x && s->get_y() == y) {
                s->directionx = dirx;
                s->directiony = diry;
                s->shell_symbol = "*";
                found = true;
                break;
            }
        }
        if (!found) {
            auto s_copy = std::make_shared<shell>(&new_board->arr[x][y], dirx, diry);
            s_copy->shell_symbol = "*";
            new_board->arr[x][y].add_Object(s_copy);
            new_board->shells.push_back(s_copy);
        }
    }

    // Place/update tanks
    for (const auto& tup : tank_data) {
        int x, y, dirx, diry;
        std::string gear;
        std::tie(x, y, dirx, diry, gear) = tup;
        tank* found_tank = nullptr;
        for (auto& t_ptr : new_board->tanks) {
            tank* t = t_ptr.get();
            if (t->get_x() == x && t->get_y() == y) {
                t->directionx = dirx;
                t->directiony = diry;
                t->gear = gear;
                found_tank = t;
                break;
            }
        }
        if (!found_tank) {
            auto t_copy = std::make_shared<tank>('1', 0, 0, dirx, diry, &new_board->arr[x][y], nullptr);
            t_copy->set_x(x);
            t_copy->set_y(y);
            t_copy->gear = gear;
            new_board->arr[x][y].add_Object(t_copy);
            new_board->tanks.push_back(t_copy);
        }
    }

    return new_board;
}

void SatelliteViewImpl::simulate_step(
    game_board& board,
    const std::tuple<int, int, std::string>& tank_command
) {
    int x, y;
    std::string action;
    std::tie(x, y, action) = tank_command;

    // Find the tank at (x, y)
    tank* target_tank = nullptr;
    for (auto& t_ptr : board.tanks) {
        tank* t = t_ptr.get();
        if (t->get_x() == x && t->get_y() == y) {
            target_tank = t;
            break;
        }
    }

    // Simulate the action for the target tank
    if (target_tank) {
        if (action == "fw") {
            target_tank->gear = "forward";
            target_tank->move_forward(board);
        } else if (action == "bw") {
            target_tank->gear = "backward";
            target_tank->move_backwards(board);
        } else if (action == "shoot") {
            target_tank->shoot(&board);
        } else if (action == "r4r") {
            target_tank->rotate_4("right");
        } else if (action == "r4l") {
            target_tank->rotate_4("left");
        } else if (action == "r8r") {
            target_tank->rotate_8("right");
        } else if (action == "r8l") {
            target_tank->rotate_8("left");
        }
    }

    // Advance the board state
    std::unordered_set<tank*> dummy_recently_killed;
    board.do_step(&dummy_recently_killed);
}