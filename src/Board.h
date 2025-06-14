#ifndef BOARD_H
#define BOARD_H

#include <memory>
#include <vector>
#include <iostream>
#include <algorithm>
#include <string>
#include "GameObject.h"
#include <unordered_set>
#include "SatelliteView.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Forward declarations
class game_object;
class tank;
class shell;
class mine;
class wall;

// ==========
// cell
// ==========
class cell {
public:
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
    game_object* get_Object();
    void add_Object(std::shared_ptr<game_object> obj);
    void remove_Object(game_object* obj);
    void print();
};

// ==========
// game_board
// ==========
class game_board {
public:
    int n;
    int m;
    std::vector<std::shared_ptr<tank>> tanks;
    std::vector<std::shared_ptr<shell>> shells;
    std::vector<std::vector<cell>> arr;
    std::vector<cell*> collisions;

    game_board(int n, int m, std::vector<std::vector<cell>> arr);

    cell& get_cell(int x, int y);

    void add_tank(std::shared_ptr<tank> t);
    void remove_tank(game_object* t);

    void add_shell(std::shared_ptr<shell> s);
    std::shared_ptr<shell> get_shared_shell(shell* s);
    void remove_shell(game_object* s);

    static std::unique_ptr<game_board> generate_board(
    SatelliteView &view,
    int n, int m,
    const std::vector<std::tuple<int, int, int, int>>& shell_data,
    const std::vector<std::tuple<int, int, int, int, std::string>>& tank_data
    );
    void simulate_step(const std::tuple<int, int, std::string>& tank_command);
    
    void print_board();
    std::unique_ptr<game_board> dummy_copy() const;
    int countAliveTanksForPlayer(char symbol) const;
    std::string get_board_state();
    
    bool do_half_step(std::unordered_set<tank*>* recently_killed);
    bool do_step(std::unordered_set<tank*>* recently_killed);
    void process_shells();
    bool handle_cell_collisions(std::unordered_set<tank*>* recently_killed);

    void simulate_step(
        const std::tuple<int, int, std::string>& tank_action
    ) const;

    void destroy_all_objects();
};

#endif // BOARD_H
