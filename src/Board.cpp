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
    std::cout << "[DEBUG] Entered game_board::print_board()" << std::endl;
    std::cout << "[DEBUG] Board dimensions: n = " << n << ", m = " << m << std::endl;
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

std::string game_board::get_board_state() {
    std::string state;
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            if (arr[i][j].has_Object()) {
                game_object* obj = arr[i][j].get_Object();
                state += obj->get_symbol();
                if (obj->symbol == 'w') {
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
        if (std::find(shells.begin(), shells.end(), s) == shells.end()) continue;
        s->shell_move_forward(*this);

        int shell_count = 0;
        for (const auto& obj : s->curcell->objects) {
            if (dynamic_cast<shell*>(obj.get())) shell_count++;
        }

        if (shell_count >= 2) {
            cell* c = s->curcell;
            for (auto it = c->objects.begin(); it != c->objects.end();) {
                if (shell* other = dynamic_cast<shell*>((*it).get())) {
                    remove_shell(other);
                    it = c->objects.erase(it);
                } else {
                    ++it;
                }
            }
            continue;
        }

        if (s->curcell->has_Object()) {
            game_object* obj = s->curcell->get_Object();
            if (obj->symbol == 'w') {
                if (wall* w = dynamic_cast<wall*>(obj)) {
                    w->hp--;
                    if (w->hp <= 0) {
                        s->curcell->remove_Object(w);
                    }
                    s->curcell->remove_Object(s.get());
                    remove_shell(s.get());
                }
            }
        }
    }
}

bool game_board::handle_cell_collisions() {
    bool game_over = false;

    for (cell* c : collisions) {
        if (c->objects.size() >= 2) {
            game_object* first_obj = c->objects[0].get();

            if (mine* m = dynamic_cast<mine*>(first_obj)) {
                for (auto it = c->objects.begin(); it != c->objects.end();) {
                    if (tank* t = dynamic_cast<tank*>((*it).get())) {
                        remove_tank(t);
                        c->remove_Object(t);
                        it = c->objects.begin();
                    } else {
                        ++it;
                    }
                }
                c->remove_Object(m);
                game_over = true;
            } else if (shell* s = dynamic_cast<shell*>(first_obj)) {
                for (auto it = c->objects.begin(); it != c->objects.end();) {
                    if (tank* t = dynamic_cast<tank*>((*it).get())) {
                        remove_tank(t);
                        c->remove_Object(t);
                        it = c->objects.begin();
                    } else {
                        ++it;
                    }
                }
                c->remove_Object(s);
                remove_shell(s);
                game_over = true;
            } else if (dynamic_cast<tank*>(first_obj)) {
                for (auto it = c->objects.begin(); it != c->objects.end();) {
                    if (dynamic_cast<shell*>((*it).get()) || dynamic_cast<mine*>((*it).get())) {
                        ++it;
                    } else {
                        if (tank* tt = dynamic_cast<tank*>((*it).get())) {
                            remove_tank(tt);
                        }
                        c->remove_Object((*it).get());
                        it = c->objects.begin();
                    }
                }
                game_over = true;
            }
        }
    }

    collisions.clear();
    return game_over;
}
