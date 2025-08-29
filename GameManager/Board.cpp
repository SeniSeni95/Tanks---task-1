#include "Board.h"
#include <memory>
#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <unordered_set>
#include "GameObject.h"
#include "../common/SatelliteView.h"
#include "../common/SymbolObject.h"

// --------------------
// cell methods
// --------------------

void cell::set_X(int x) { this->x = x; }
void cell::set_Y(int y) { this->y = y; }
int cell::get_X() { return x; }
int cell::get_Y() { return y; }

bool cell::has_Object() {
    return !objects.empty();
}

game_object* cell::get_Object() {
    return objects.empty() ? nullptr : objects[0].get();
}

void cell::add_Object(std::shared_ptr<game_object> obj) {
    objects.push_back(obj);
}

void cell::remove_Object(game_object* obj) {
    auto it = std::remove_if(objects.begin(), objects.end(),
        [obj](const std::shared_ptr<game_object>& ptr) {
            return ptr.get() == obj;
        });
    objects.erase(it, objects.end());
}

void cell::print() {
    // std::cout << "[DEBUG] cell::print() at (" << x << "," << y << "), objects.size() = " << objects.size() << std::endl;

    if (!objects.empty() && objects[0]) {
        // std::cout << "[DEBUG] Calling objects[0]->print() at (" << x << "," << y << ")" << std::endl;
        objects[0]->print();
    } else {
        std::cout << "[  ]";
    }
}


// --------------------
// game_board methods
// --------------------

game_board::game_board(int n, int m, std::vector<std::vector<cell>> arr)
    : n(n), m(m), arr(std::move(arr)) {}

cell& game_board::get_cell(int x, int y) {
    return arr[x][y];
}

void game_board::add_tank(std::shared_ptr<tank> t) {
    tanks.push_back(std::move(t));
}

void game_board::remove_tank(game_object* t) {
    if (tank* tk = dynamic_cast<tank*>(t)) {
        tk->alive = false; 
    }

    tanks.erase(std::remove_if(tanks.begin(), tanks.end(),
        [t](const std::shared_ptr<tank>& ptr) { return ptr.get() == t; }),
        tanks.end());
}


void game_board::add_shell(std::shared_ptr<shell> s) {
    shells.insert(shells.begin(), std::move(s));
}

void game_board::remove_shell(game_object* s) {
    shells.erase(std::remove_if(shells.begin(), shells.end(),
        [s](const std::shared_ptr<shell>& ptr) { return ptr.get() == s; }),
        shells.end());
}

void game_board::print_board() {
    // std::cout << "[DEBUG] Entered game_board::print_board()" << std::endl;
    // std::cout << "[DEBUG] Board dimensions: n = " << n << ", m = " << m << std::endl;
    try {
        for (int j = 0; j < m; ++j) {
            for (int i = 0; i < n; ++i) {
                get_cell(i, j).print();
            }
            std::cout << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "[ERROR] Exception in print_board(): " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "[ERROR] Unknown exception in print_board()" << std::endl;
    }
}


std::shared_ptr<shell> game_board::get_shared_shell(shell* s) {
    for (auto& sp : shells) {
        if (sp.get() == s) return sp;
    }
    throw std::runtime_error("Shell not found in board->shells");
}

std::string game_board::get_board_state() {
    std::string state;
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            if (get_cell(i, j).has_Object()) {
                game_object* obj = get_cell(i, j).get_Object();
                state += obj->get_symbol();
                if (obj->get_symbol() == '#') {
                    wall* w = dynamic_cast<wall*>(obj);
                    state += std::to_string(w->hp);
                }
            } else {
                state += ' ';
            }
        }
        state += '\n';
    }
    for (const auto& t : tanks) {
        state += std::to_string(t->shot_timer);
        state += std::to_string(t->shells);
        state += t->gear;
    }
    for (const auto& s : shells) {
        state += std::to_string(s->just_created);
    }
    return state;
}

std::unique_ptr<game_board> game_board::symbol_copy() const {
    // Build empty board grid
    std::vector<std::vector<cell>> arr_copy;
    arr_copy.reserve(n);
    for (int i = 0; i < n; ++i) {
        std::vector<cell> row;
        row.reserve(m);
        for (int j = 0; j < m; ++j) {
            row.emplace_back(i, j);
        }
        arr_copy.push_back(std::move(row));
    }

    auto new_board = std::make_unique<game_board>(n, m, std::move(arr_copy));

    // Copy only symbols into SymbolObjects
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            const auto& src_cell = arr[i][j];
            auto& dst_cell = new_board->get_cell(i, j);

            for (const auto& obj_ptr : src_cell.objects) {
                char s = obj_ptr->get_symbol();
                auto clone = std::make_shared<SymbolObject>(i, j, s);
                dst_cell.add_Object(clone);
            }
        }
    }

    return new_board;
}

std::unique_ptr<game_board> game_board::dummy_copy() const {
    std::vector<std::vector<cell>> arr_copy;
    arr_copy.reserve(n);
    for (int j = 0; j < n; ++j) {
        std::vector<cell> col;
        col.reserve(m);
        for (int i = 0; i < m; ++i) {
            col.emplace_back(j, i);
        }
        arr_copy.push_back(std::move(col));
    }

    auto new_board = std::make_unique<game_board>(n, m, std::move(arr_copy));

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            const auto& src_cell = arr[i][j];
            auto& dst_cell = new_board->get_cell(i, j);

            for (const auto& obj_ptr : src_cell.objects) {
                if (auto t = dynamic_cast<tank*>(obj_ptr.get())) {
                    auto t_copy = std::make_shared<tank>(
                        t->symbol, t->player_number, t->tank_number,
                        t->directionx, t->directiony, &dst_cell, nullptr
                    );
                    t_copy->shells = t->shells;
                    t_copy->shot_timer = t->shot_timer;
                    t_copy->cannon_symbol = t->cannon_symbol;
                    t_copy->gear = t->gear;
                    t_copy->alive = t->alive;
                    t_copy->set_x(t->get_x());
                    t_copy->set_y(t->get_y());
                    dst_cell.add_Object(t_copy);
                    new_board->tanks.push_back(t_copy);
                } else if (auto s = dynamic_cast<shell*>(obj_ptr.get())) {
                    auto s_copy = std::make_shared<shell>(&dst_cell, s->directionx, s->directiony);
                    s_copy->shell_symbol = "*"; 
                    s_copy->just_created = s->just_created;
                    dst_cell.add_Object(s_copy);
                    new_board->shells.push_back(s_copy);
                } else if (auto m = dynamic_cast<mine*>(obj_ptr.get())) {
                    auto m_copy = std::make_shared<mine>(m->get_symbol(), &dst_cell);
                    dst_cell.add_Object(m_copy);
                } else if (auto w = dynamic_cast<wall*>(obj_ptr.get())) {
                    auto w_copy = std::make_shared<wall>(w->get_symbol(), &dst_cell);
                    w_copy->hp = w->hp;
                    dst_cell.add_Object(w_copy);
                }
            }
        }
    }

    return new_board;
}

bool game_board::do_half_step(std::unordered_set<tank*>* recently_killed) {
    bool game_over = false;
    process_shells();
    if (!collisions.empty()) {
        game_over = handle_cell_collisions(recently_killed);
    }
    return game_over;
}

bool game_board::do_step(std::unordered_set<tank*>* recently_killed) {
    bool game_over = do_half_step(recently_killed);
    if (game_over) return true;
    return do_half_step(recently_killed);
}

void game_board::process_shells() {
    std::vector<std::shared_ptr<shell>> shells_to_process = shells;

    for (auto& s : shells_to_process) {
        // Skip if this shell was already removed
        if (std::find(shells.begin(), shells.end(), s) == shells.end())
            continue;

        s->shell_move_forward(*this);
        cell* c = s->curcell;

        // Check for direct shell-shell overlap
        int shell_count = 0;
        for (const auto& obj : c->objects) {
            if (dynamic_cast<shell*>(obj.get())) {
                shell_count++;
            }
        }

        // If multiple shells collide, remove them immediately
        if (shell_count >= 2) {
            for (auto it = c->objects.begin(); it != c->objects.end();) {
                if (shell* sh = dynamic_cast<shell*>((*it).get())) {
                    remove_shell(sh);
                    it = c->objects.erase(it);
                } else {
                    ++it;
                }
            }
            continue;
        }

        // Check for wall collision
        for (const auto& obj : c->objects) {
            if (obj.get() == s.get()) continue;
            if (wall* w = dynamic_cast<wall*>(obj.get())) {
                w->hp--;
                if (w->hp <= 0) {
                    c->remove_Object(w);
                }
                c->remove_Object(s.get());
                remove_shell(s.get());
                goto continue_loop;
            }
        }

        // Check for tank or other collision
        for (const auto& obj : c->objects) {
            if (obj.get() == s.get()) continue;
            if (dynamic_cast<tank*>(obj.get()) || dynamic_cast<mine*>(obj.get())) {
                if (std::find(collisions.begin(), collisions.end(), c) == collisions.end()) {
                    collisions.push_back(c);
                }
                break;
            }
        }

    continue_loop:
        continue;
    }
}



bool game_board::handle_cell_collisions(std::unordered_set<tank*>* recently_killed) {
    for (cell* c : collisions) {
        std::vector<game_object*> tanks_to_remove;
        std::vector<game_object*> shells_to_remove;
        std::vector<game_object*> mines_to_remove;

        for (const auto& ptr : c->objects) {
            game_object* obj = ptr.get();
            if (dynamic_cast<tank*>(obj)) {
                tanks_to_remove.push_back(obj);
            } else if (dynamic_cast<shell*>(obj)) {
                shells_to_remove.push_back(obj);
            } else if (dynamic_cast<mine*>(obj)) {
                mines_to_remove.push_back(obj);
            }
        }

        // --- TANK VS TANK: destroy all tanks if more than one tank on the cell ---
        if (tanks_to_remove.size() > 1) {
            for (game_object* t : tanks_to_remove) {
                if (tank* tk = dynamic_cast<tank*>(t)) {
                    if (tk->alive && recently_killed) {
                        recently_killed->insert(tk);
                    }
                    tk->alive = false;
                    remove_tank(tk);
                    c->remove_Object(tk);
                }
            }
        }
        // --- TANK VS SHELL or TANK VS MINE: destroy tanks if at least one tank and one shell or mine ---
        else if ((!shells_to_remove.empty() && !tanks_to_remove.empty()) ||
                 (!mines_to_remove.empty() && !tanks_to_remove.empty())) {
            // Destroy tanks
            for (game_object* t : tanks_to_remove) {
                if (tank* tk = dynamic_cast<tank*>(t)) {
                    if (tk->alive && recently_killed) {
                        recently_killed->insert(tk);
                    }
                    tk->alive = false;
                    remove_tank(tk);
                    c->remove_Object(tk);
                }
            }
            // Destroy shells ONLY if a tank is present (tank vs shell)
            if (!shells_to_remove.empty() && !tanks_to_remove.empty()) {
                for (game_object* s : shells_to_remove) {
                    remove_shell(s);
                    c->remove_Object(s);
                }
            }
        }
    }

    collisions.clear();

    // Count living tanks per player
    bool p1_alive = false;
    bool p2_alive = false;
    for (const auto& t : tanks) {
        if (!t->alive) continue;
        if (t->symbol == '1') p1_alive = true;
        if (t->symbol == '2') p2_alive = true;
    }

    return !(p1_alive && p2_alive);
}



int game_board::countAliveTanksForPlayer(char symbol) const {
    int count = 0;
    for (const auto& t_ptr : tanks) {
        if (t_ptr->alive && t_ptr->symbol == symbol) {
            ++count;
        }
    }
    return count;
}

std::unique_ptr<game_board> game_board::generate_board(
    SatelliteView &view,
    int n,
    int m,
    const std::vector<std::tuple<int, int, int, int>>& shell_data,
    const std::vector<std::tuple<int, int, int, int, std::string>>& tank_data
) {
    std::vector<std::vector<cell>> arr;
    arr.reserve(n);
    for (int j = 0; j < n; ++j) {
        std::vector<cell> col;
        col.reserve(m);
        for (int i = 0; i < m; ++i) {
            col.emplace_back(j, i);
        }
        arr.push_back(std::move(col));
    }

    auto new_board = std::make_unique<game_board>(n, m, std::move(arr));
    std::vector<int> tank_counters(2, 0); // count tanks per player

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            char ch = view.getObjectAt(i, j);
            cell& current = new_board->get_cell(i, j);

            switch (ch) {
                case '#':
                    current.add_Object(std::make_shared<wall>('#', &current));
                    break;
                case '@':
                    current.add_Object(std::make_shared<mine>('@', &current));
                    break;
                case '*': {
                    // Find matching shell data
                    auto it = std::find_if(shell_data.begin(), shell_data.end(),
                        [i, j](const auto& t) {
                            return std::get<0>(t) == i && std::get<1>(t) == j;
                        });

                    int dx = 0, dy = 0;
                    if (it != shell_data.end()) {
                        dx = std::get<2>(*it);
                        dy = std::get<3>(*it);
                    }

                    auto s_ptr = std::make_shared<shell>(&current, dx, dy);
                    s_ptr->shell_symbol = "*";
                    new_board->shells.push_back(s_ptr);
                    current.add_Object(std::move(s_ptr));
                    break;
                }
                case '1':
                case '2': {
                    int player_index = (ch == '1') ? 0 : 1;
                    int tank_number = ++tank_counters[player_index];

                    // Find matching tank data
                    auto it = std::find_if(tank_data.begin(), tank_data.end(),
                        [i, j](const auto& t) {
                            return std::get<0>(t) == i && std::get<1>(t) == j;
                        });

                    int dx = (player_index == 0 ? -1 : 1), dy = 0;
                    std::string gear = "forward";

                    if (it != tank_data.end()) {
                        dx = std::get<2>(*it);
                        dy = std::get<3>(*it);
                        gear = std::get<4>(*it);
                    }

                    auto t_ptr = std::make_shared<tank>(
                        ch, player_index + 1, tank_number,
                        dx, dy, &current, nullptr
                    );
                    t_ptr->set_x(i);
                    t_ptr->set_y(j);
                    t_ptr->gear = gear;
                    new_board->tanks.push_back(t_ptr);
                    current.add_Object(t_ptr);
                    break;
                }
                default:
                    break;
            }
        }
    }

    return new_board;
}

void game_board::simulate_step(const std::tuple<int, int, std::string>& tank_command) {
    int x, y;
    std::string action;
    std::tie(x, y, action) = tank_command;

    // Find the tank at (x, y)
    tank* target_tank = nullptr;
    for (auto& t_ptr : tanks) {
        tank* t = t_ptr.get();
        if (t->get_x() == x && t->get_y() == y) {
            target_tank = t;
            break;
        }
    }

    // Simulate the action for the target tank
    if (target_tank) {
        target_tank->turn(this, action);
    }

    // Advance the board state
    std::unordered_set<tank*> dummy_recently_killed;
    do_step(&dummy_recently_killed);
}

