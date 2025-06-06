#include "board.h"
#include <memory>
#include <algorithm>
#include <iostream>
#include <stdexcept>

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
    objects.push_back(std::move(obj));
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

void game_board::add_tank(std::shared_ptr<tank> t) {
    tanks.push_back(std::move(t));
}

void game_board::remove_tank(game_object* t) {
    if (tank* tk = dynamic_cast<tank*>(t)) {
        tk->alive = false; // ✅ Safety in case called directly
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
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < m; ++j) {
                arr[i][j].print();
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
            if (arr[i][j].has_Object()) {
                game_object* obj = arr[i][j].get_Object();
                state += obj->get_symbol();
                if (obj->get_symbol() == 'w') {
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

std::unique_ptr<game_board> game_board::deep_copy() const {
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

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            const auto& src_cell = arr[i][j];
            auto& dst_cell = new_board->arr[i][j];

            for (const auto& obj_ptr : src_cell.objects) {
                game_object* obj = obj_ptr.get();
                if (tank* t = dynamic_cast<tank*>(obj)) {
                    auto new_tank = std::make_shared<tank>(*t);
                    new_board->tanks.push_back(new_tank);
                    dst_cell.add_Object(new_tank);
                } else if (shell* s = dynamic_cast<shell*>(obj)) {
                    auto new_shell = std::make_shared<shell>(*s);
                    new_board->shells.push_back(new_shell);
                    dst_cell.add_Object(new_shell);
                } else if (mine* m = dynamic_cast<mine*>(obj)) {
                    dst_cell.add_Object(std::make_shared<mine>(*m));
                } else if (wall* w = dynamic_cast<wall*>(obj)) {
                    dst_cell.add_Object(std::make_shared<wall>(*w));
                }
            }
        }
    }

    return new_board;
}

bool game_board::do_half_step() {
    bool game_over = false;
    process_shells();
    if (!collisions.empty()) {
        game_over = handle_cell_collisions();
    }
    return game_over;
}

bool game_board::do_step() {
    bool game_over = do_half_step();
    if (game_over) return true;
    return do_half_step();
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

        // Check for tank or other collision — defer to collision handler
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



bool game_board::handle_cell_collisions() {
    // std::cout << "[DEBUG] Entering handle_cell_collisions..." << std::endl;

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

        // Remove all tanks hit by shell or mine
        if ((!shells_to_remove.empty() && !tanks_to_remove.empty()) ||
            (!mines_to_remove.empty() && !tanks_to_remove.empty())) {
            // std::cout << "[DEBUG] Tank(s) killed in collision at cell!" << std::endl;
            for (game_object* t : tanks_to_remove) {
                if (tank* tk = dynamic_cast<tank*>(t)) {
                    // std::cout << "[DEBUG] Marking tank " << tk->symbol << " as dead." << std::endl;
                    tk->alive = false;
                    remove_tank(tk);
                    c->remove_Object(tk);
                }
            }
        }

        for (game_object* m : mines_to_remove) {
            // std::cout << "[DEBUG] Removing mine." << std::endl;
            c->remove_Object(m);
        }

        for (game_object* s : shells_to_remove) {
            // std::cout << "[DEBUG] Removing shell." << std::endl;
            remove_shell(s);
            c->remove_Object(s);
        }
    }

    collisions.clear();

    // Count living tanks per player
    bool p1_alive = false;
    bool p2_alive = false;

    // std::cout << "[DEBUG] Tanks after collision handling:" << std::endl;
    for (const auto& t : tanks) {
        // std::cout << "  Tank " << t->symbol << " alive=" << t->alive << std::endl;

        if (!t->alive) continue;

        if (t->symbol == '1') p1_alive = true;
        if (t->symbol == '2') p2_alive = true;
    }

    bool result = !(p1_alive && p2_alive);
    // std::cout << "[DEBUG] handle_cell_collisions returning " << std::boolalpha << result << std::endl;
    return result;
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

