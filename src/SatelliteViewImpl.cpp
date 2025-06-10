#include "SatelliteViewImpl.h"
#include "GameObject.h"

std::unique_ptr<SatelliteViewImpl> SatelliteViewImpl::simulate_step(
    const std::vector<std::tuple<int, int, int, int>>& shell_data,
    const std::vector<std::tuple<int, int, int, int, std::string>>& tank_data,
    const std::string& action
) const {
    // 1. Deep copy the board
    auto new_board = std::unique_ptr<game_board>(boardCopy->dummy_copy());

    // 2. Place/update shells
    for (const auto& shell_info : shell_data) {
        int x, y, dirx, diry;
        std::tie(x, y, dirx, diry) = shell_info;

        // Try to find an existing shell at (x, y)
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
            // Create a new shell at (x, y) with the given direction
            auto s_copy = std::make_shared<shell>(&new_board->arr[x][y], dirx, diry);
            s_copy->shell_symbol = "*";
            new_board->arr[x][y].add_Object(s_copy);
            new_board->shells.push_back(s_copy);
        }
    }

    // 3. Place/update tanks
    tank* target_tank = nullptr;
    for (const auto& tup : tank_data) {
        int x, y, dirx, diry;
        std::string gear;
        std::tie(x, y, dirx, diry, gear) = tup;

        // Try to find an existing tank at (x, y)
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
            // Create a new tank at (x, y) with the given direction and gear
            auto t_copy = std::make_shared<tank>('1', 0, 0, dirx, diry, &new_board->arr[x][y], nullptr);
            t_copy->set_x(x);
            t_copy->set_y(y);
            t_copy->gear = gear;
            new_board->arr[x][y].add_Object(t_copy);
            new_board->tanks.push_back(t_copy);
            found_tank = t_copy.get();
        }
        // Assume the first tank in the list is the one to act
        if (!target_tank) target_tank = found_tank;
    }

    // 4. Simulate the action for the target tank
    if (target_tank) {
        if (action == "fw") {
            target_tank->gear = "forward";
            target_tank->move_forward(*new_board);
        } else if (action == "bw") {
            target_tank->gear = "backward";
            target_tank->move_backwards(*new_board);
        } else if (action == "shoot") {
            target_tank->shoot(new_board.get());
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

    // 5. Advance the board state
    std::unordered_set<tank*> dummy_recently_killed;
    new_board->do_step(&dummy_recently_killed);

    // 6. Return a new SatelliteViewImpl with the updated board
    auto result = std::make_unique<SatelliteViewImpl>();
    result->updateCopy(*new_board);
    return result;
}