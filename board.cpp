#include <iostream>
#include <vector>
#include <string>
#include <math.h>
#include <algorithm>
#include "board.h"
#include "game_objects.h"
using namespace std;
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Forward declarations
struct game_object;
struct tank;
struct shell;
struct mine;
struct wall;

struct cell;
struct game_board;

struct cell {
    int x;
    int y;
    vector<game_object*> objects; // Vector to store multiple game_object pointers

public:
    cell(int x, int y) {
        this->x = x;
        this->y = y;
    }   
    cell(int x, int y, game_object* obj) {
        this->x = x;
        this->y = y;
        this->objects.push_back(obj); // Add the object to the vector
    }   
    cell() {
        this->x = 0;
        this->y = 0;
    } 

    void set_X(int x) {
        this->x = x;
    }
    void set_Y(int y) {
        this->y = y;
    }
    int get_X() {
        return x;
    }
    int get_Y() {
        return y;
    }
    bool has_Object() {
        return !objects.empty(); // Check if the vector is not empty
    }
    game_object* get_Object() {
        return objects[0]; // Return the vector of objects
    }
    void add_Object(game_object* obj) {
        this->objects.push_back(obj); // Add an object to the vector
    }
    void remove_Object(game_object* obj) {
        // Remove the specific object from the vector
        objects.erase(remove(objects.begin(), objects.end(), obj), objects.end());
    }
    void print() {
        if (!objects.empty()) {
            objects[0]->print(); // Call the print method of the first object
        } else {
            cout << "[  ]"; // Print an empty cell if no objects are present
        }
    }
};

struct game_board {
    int n; // number of rows
    int m; // number of columns
    vector<tank*> tanks;
    vector<shell*> shells;
    vector<vector<cell>> arr; // 2D array to represent the game board
    vector<cell*> collisions;

    game_board(int n, int m, vector<vector<cell>> arr) {
        this->n = n;
        this->m = m;
        this->arr = arr; // Initialize the game board with the provided 2D vector
    }
    

    void add_tank(tank* t) {
        tanks.push_back(t); // Add a tank to the tanks vector
    }

    void remove_tank(tank* t) {
        tanks.erase(remove(tanks.begin(), tanks.end(), t), tanks.end()); // Remove a tank
    }

    void add_shell(shell* s) {
        shells.push_back(s); // Add a shell to the shells vector
    }

    void remove_shell(shell* s) {
        shells.erase(remove(shells.begin(), shells.end(), s), shells.end()); // Remove a shell
    }

    void print_board() {
        // Function to print the game board
        for (int i = 0 ; i < n; i++) {  // Flip vertical
            for (int j = 0; j < m; ++j) {
                arr[i][j].print(); // Call the print method of each cell
            }
            cout << endl;
        }
    }

    game_board* deep_copy() {
        game_board* new_board = new game_board(n, m, arr);

        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < m; ++j) {
                cell* c = &arr[i][j];
                vector<game_object*> old_objects = c->objects;
                c->objects.clear(); // Clear the old objects in the new board

                for (game_object* obj : old_objects) {
                    if (tank* t = dynamic_cast<tank*>(obj)) {
                        new_board->add_tank(new tank(t->symbol, t->directiony, t->directionx, c));
                    } else if (shell* s = dynamic_cast<shell*>(obj)) {
                        new_board->add_shell(new shell(c, s->directionx, s->directiony));
                    } else if (mine* m = dynamic_cast<mine*>(obj)) {
                        new mine(m -> symbol, c);
                    } else if (wall* w = dynamic_cast<wall*>(obj)) {
                        new wall(w->symbol, c);
                    }
                }
            }
        }

        // Copy collisions array
        vector<cell*> new_collisions;
        for (cell* c : collisions) {
            new_collisions.push_back(&new_board->arr[c->x][c->y]);
        }
        new_board->collisions = new_collisions;

        return new_board;
    }

    bool do_half_step() {
        bool game_over = false;

        process_shells();
        // cout << "Done pro 1" << endl;
        if (!collisions.empty()) {
            game_over = handle_cell_collisions(); // Handle cell collisions
        }

        return game_over;
    }

    bool do_step() {
        bool game_over = do_half_step();
        if (game_over) {
            return true;
        }

        return do_half_step();
    }

    void process_shells() {
        // Copy of the shells list to avoid issues while deleting in-place
        vector<shell*> shells_to_process = shells;
    
        for (shell* s : shells_to_process) {
            // Check if the shell still exists in the board's shell list
            if (find(shells.begin(), shells.end(), s) == shells.end()) {
                continue; // Skip this shell if it has already been removed
            }
    
            // Move the shell forward
            s->shell_move_forward(*this);
    
            // Count how many shells are in this cell
            int shell_count = 0;
            for (game_object* obj : s->curcell->objects) {
                if (dynamic_cast<shell*>(obj)) {
                    shell_count++;
                }
            }
            cout << shell_count << " shell(s) in cell " << s->curcell->x << "," << s->curcell->y << endl;
    
            // If two or more shells are in the same cell, destroy all of them
            if (shell_count >= 2) {
                cout << "Handling shells in cell (" << s->curcell->x << ", " << s->curcell->y << ")" << endl;
    
                // Safely iterate over the objects in the cell and remove all shells
                for (auto it = s->curcell->objects.begin(); it != s->curcell->objects.end();) {
                    game_object* obj = *it;
                    if (shell* other = dynamic_cast<shell*>(obj)) {
                        cout << "Removing shell at (" << other->x << ", " << other->y << ")" << endl;
    
                        // Remove the shell from the board's shell list
                        remove_shell(other);
    
                        // Remove the shell from the cell's objects vector
                        it = s->curcell->objects.erase(it);
    
                        // Delete the shell
                        delete other;
                    } else {
                        ++it; // Skip non-shell objects
                    }
                }
                continue; // Skip remaining logic since this shell was destroyed
            }
    
            // If the cell contains an object and it's a wall, handle wall collision
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

    bool handle_cell_collisions() {
        bool game_over = false; // Flag to indicate if the game is over

        for (cell* c : collisions) {
            if (c->objects.size() >= 2) {
                cout << "Collision detected in cell (" << c->x << ", " << c->y << ")!" << endl;
    
                game_object* first_obj = c->objects[0];
    
                if (mine* m = dynamic_cast<mine*>(first_obj)) {
                    cout << "Mine detected in collision!" << endl;
                    for (auto it = c->objects.begin(); it != c->objects.end();) {
                        game_object* obj = *it;
                        if (tank* t = dynamic_cast<tank*>(obj)) {
                            cout << "Destroying tank " << t->symbol << " due to mine!" << endl;
                            remove_tank(t);
                            c->remove_Object(t);
                            delete t;
                            it = c->objects.begin(); // Restart after modification
                        } else {
                            ++it;
                        }
                    }
                    c->remove_Object(m);
                    delete m;
                    game_over = true;
                }
    
                else if (shell* s = dynamic_cast<shell*>(first_obj)) {
                    cout << "Shell detected in collision!" << endl;
                    for (auto it = c->objects.begin(); it != c->objects.end();) {
                        game_object* obj = *it;
                        if (tank* t = dynamic_cast<tank*>(obj)) {
                            cout << "Destroying tank " << t->symbol << " due to shell!" << endl;
                            remove_tank(t);
                            c->remove_Object(t);
                            delete t;
                            it = c->objects.begin(); // Restart after modification
                        } else {
                            ++it;
                        }
                    }
                    c->remove_Object(s);
                    remove_shell(s);
                    delete s;
                    game_over = true;
                }
    
                else if (tank* t = dynamic_cast<tank*>(first_obj)) {
                    cout << "Tank detected in collision!" << endl;
                    for (auto it = c->objects.begin(); it != c->objects.end();) {
                        game_object* obj = *it;
                        if (dynamic_cast<shell*>(obj) || dynamic_cast<mine*>(obj)) {
                            ++it;
                        } else {
                            cout << "Destroying object with symbol: " << obj->symbol << endl;
                            if (tank* tt = dynamic_cast<tank*>(obj)) {
                                remove_tank(tt);
                            }
                            c->remove_Object(obj);
                            delete obj;
                            it = c->objects.begin(); // Restart after modification
                        }
                    }
                    game_over = true;
                }
            }
        }
    
        // Clear collisions list after all processing
        collisions.clear();

        return game_over; // Return the game over status
    }
};