#include "board.h"

cell::cell(int x, int y) {
    this->x = x;
    this->y = y;
}

cell::cell(int x, int y, game_object* obj) {
    this->x = x;
    this->y = y;
    this->objects.push_back(obj); // Add the object to the vector
}

cell::cell() {
    this->x = 0;
    this->y = 0;
}

void cell::set_X(int x) {
    this->x = x;
}

void cell::set_Y(int y) {
    this->y = y;
}

int cell::get_X() {
    return x;
}

int cell::get_Y() {
    return y;
}

bool cell::has_Object() {
    return !objects.empty(); // Check if the vector is not empty
}

game_object* cell::get_Object() {
    return objects[0]; // Return the first object in the vector
}

void cell::add_Object(game_object* obj) {
    this->objects.push_back(obj); // Add an object to the vector
}

void cell::remove_Object(game_object* obj) {
    // Remove the specific object from the vector
    objects.erase(std::remove(objects.begin(), objects.end(), obj), objects.end());
}

void cell::print() {
    if (!objects.empty()) {
        objects[0]->print(); // Call the print method of the first object
    } else {
        std::cout << "[  ]"; // Print an empty cell if no objects are present
    }
}

game_board::game_board(int n, int m, std::vector<std::vector<cell>> arr) {
    this->n = n;
    this->m = m;
    this->arr = arr; // Initialize the game board with the provided 2D vector
}

void game_board::add_tank(tank* t) {
    tanks.push_back(t); // Add a tank to the tanks vector
}

void game_board::remove_tank(tank* t) {
    tanks.erase(std::remove(tanks.begin(), tanks.end(), t), tanks.end()); // Remove a tank
}

void game_board::add_shell(shell* s) {
    shells.insert(shells.begin(), s); // Add a shell to the shells vector
}

void game_board::remove_shell(shell* s) {
    shells.erase(std::remove(shells.begin(), shells.end(), s), shells.end()); // Remove a shell
}

void game_board::print_board() {
    // Function to print the game board
    for (int i = 0; i < n; i++) { // Flip vertical
        for (int j = 0; j < m; ++j) {
            arr[i][j].print(); // Call the print method of each cell
        }
        std::cout << std::endl;
    }
}

string game_board::get_board_state() {
    std::string state;
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            if (arr[i][j].has_Object()) {
                game_object* obj = arr[i][j].get_Object();
                state += obj->get_symbol(); // Append the symbol of the object
                if (obj->symbol == 'w') {
                    wall* w = dynamic_cast<wall*>(obj);
                    state += std::to_string(w->hp); // Append the HP of the wall
                }
            } else {
                state += ' '; // Append a space if no object is present
            }
        }
        state += '\n'; // New line after each row
    }

    for (tank* t : tanks) {
        state += to_string(t->shot_timer);
        state += to_string(t->shells);
        state += t->gear;
    }

    for (shell* s : shells) {
        state += to_string(s->just_created);
    }

    return state;
}

game_board* game_board::deep_copy() {
    game_board* new_board = new game_board(n, m, arr);

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            cell* c = &(new_board->arr[i][j]);
            std::vector<game_object*> old_objects = c->objects;
            c->objects.clear(); // Clear the old objects in the new board

            for (game_object* obj : old_objects) {
                if (tank* t = dynamic_cast<tank*>(obj)) {
                    tank* new_tank = new tank(t->symbol, t->directionx, t->directiony, c, t->algo);
                    new_board->add_tank(new_tank);
                } else if (shell* s = dynamic_cast<shell*>(obj)) {
                    shell* new_shell = new shell(c, s->directionx, s->directiony);
                    new_board->add_shell(new_shell);
                } else if (mine* m = dynamic_cast<mine*>(obj)) {
                    new mine(m->symbol, c);
                } else if (wall* w = dynamic_cast<wall*>(obj)) {
                    new wall(w->symbol, c);
                }
            }
        }
    }

    // Copy collisions array
    std::vector<cell*> new_collisions;
    for (cell* c : collisions) {
        new_collisions.push_back(&new_board->arr[c->x][c->y]);
    }
    new_board->collisions = new_collisions;

    return new_board;
}

bool game_board::do_half_step() {
    bool game_over = false;

    process_shells();
    if (!collisions.empty()) {
        game_over = handle_cell_collisions(); // Handle cell collisions
    }

    return game_over;
}

bool game_board::do_step() {
    bool game_over = do_half_step();
    if (game_over) {
        return true;
    }

    return do_half_step();
}

void game_board::process_shells() {
    std::vector<shell*> shells_to_process = shells;

    for (shell* s : shells_to_process) {
        if (std::find(shells.begin(), shells.end(), s) == shells.end()) {
            continue; // Skip this shell if it has already been removed
        }

        s->shell_move_forward(*this);

        int shell_count = 0;
        for (game_object* obj : s->curcell->objects) {
            if (dynamic_cast<shell*>(obj)) {
                shell_count++;
            }
        }

        if (shell_count >= 2) {
            cell* c = s->curcell;
            for (auto it = c->objects.begin(); it != c->objects.end();) {
                game_object* obj = *it;
                if (shell* other = dynamic_cast<shell*>(obj)) {
                    remove_shell(other);
                    it = c->objects.erase(it);
                    delete other;
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
                        delete w;
                    }
                    s->curcell->remove_Object(s);
                    remove_shell(s);
                    delete s;
                }
            }
        }
    }
}

bool game_board::handle_cell_collisions() {
    bool game_over = false;

    for (cell* c : collisions) {
        if (c->objects.size() >= 2) {
            game_object* first_obj = c->objects[0];

            if (mine* m = dynamic_cast<mine*>(first_obj)) {
                for (auto it = c->objects.begin(); it != c->objects.end();) {
                    game_object* obj = *it;
                    if (tank* t = dynamic_cast<tank*>(obj)) {
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
                    game_object* obj = *it;
                    if (tank* t = dynamic_cast<tank*>(obj)) {
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
            } else if (tank* _ = dynamic_cast<tank*>(first_obj)) {
                for (auto it = c->objects.begin(); it != c->objects.end();) {
                    game_object* obj = *it;
                    if (dynamic_cast<shell*>(obj) || dynamic_cast<mine*>(obj)) {
                        ++it;
                    } else {
                        if (tank* tt = dynamic_cast<tank*>(obj)) {
                            remove_tank(tt);
                        }
                        c->remove_Object(obj);
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

void game_board::destroy_all_objects() {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            for (game_object* obj : arr[i][j].objects) {
                delete obj;
            }
            arr[i][j].objects.clear(); // Clear the objects in each cell
        }
    }

    tanks.clear();
    shells.clear();
}
