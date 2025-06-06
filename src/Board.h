#ifndef BOARD_H
#define BOARD_H

#include <memory>
#include <vector>
#include <iostream>
#include <algorithm>
#include <string>
#include "GameObject.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Forward declarations
struct game_object;
struct tank;
struct shell;
struct mine;
struct wall;

// ==========
// cell
// ==========
struct cell {
    int x, y;
    std::vector<std::shared_ptr<game_object>> objects;

    cell() : x(0), y(0) {}
    cell(int x, int y) : x(x), y(y) {}
    cell(int x, int y, std::shared_ptr<game_object> obj) : x(x), y(y) {
        objects.push_back(std::move(obj));
    }

    // Disable copy, enable move
    cell(const cell&) = delete;
    cell& operator=(const cell&) = delete;

    cell(cell&& other) noexcept
        : x(other.x), y(other.y), objects(std::move(other.objects)) {}

    cell& operator=(cell&& other) noexcept {
        if (this != &other) {
            x = other.x;
            y = other.y;
            objects = std::move(other.objects);
        }
        return *this;
    }

    void set_X(int x);
    void set_Y(int y);
    int get_X();
    int get_Y();
    bool has_Object();
    game_object* get_Object(); // returns raw pointer, safe for lookup
    void add_Object(std::shared_ptr<game_object> obj); // now uses shared_ptr
    void remove_Object(game_object* obj); // still uses raw pointer for filtering
    void print();
};

// ==========
// game_board
// ==========
struct game_board {
    int n; // rows
    int m; // cols
    std::vector<std::shared_ptr<tank>> tanks;   // owns references, not objects
    std::vector<std::shared_ptr<shell>> shells; // owns references, not objects
    std::vector<std::vector<cell>> arr;
    std::vector<cell*> collisions;

    game_board(int n, int m, std::vector<std::vector<cell>> arr);

    void add_tank(std::shared_ptr<tank> t);
    void remove_tank(game_object* t); // we can remove via pointer matching

    void add_shell(std::shared_ptr<shell> s);
    void remove_shell(game_object* s); // same for shells

    void print_board();
    std::unique_ptr<game_board> deep_copy() const;

    std::string get_board_state();

    bool do_half_step();
    bool do_step();
    void process_shells();
    bool handle_cell_collisions();

    void destroy_all_objects();
};

#endif // BOARD_H
