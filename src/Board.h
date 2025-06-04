#ifndef BOARD_H
#define BOARD_H

#include <vector>
#include <iostream>
#include <algorithm>
#include "GameObject.h"
using namespace std;

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

struct game_object; // Forward declaration of game_object
struct tank;        // Forward declaration of tank
struct shell;       // Forward declaration of shell
struct mine;        // Forward declaration of mine
struct wall;        // Forward declaration of wall

struct cell {
    int x;
    int y;
    std::vector<game_object*> objects; // Vector to store multiple game_object pointers

    cell(int x, int y);
    cell(int x, int y, game_object* obj);
    cell();


    void set_X(int x);
    void set_Y(int y);
    int get_X();
    int get_Y();
    bool has_Object();
    game_object* get_Object();
    void add_Object(game_object* obj);
    void remove_Object(game_object* obj);
    void print();
};

struct game_board {
    int n; // number of rows
    int m; // number of columns
    std::vector<tank*> tanks;
    std::vector<shell*> shells;
    std::vector<std::vector<cell>> arr; // 2D array to represent the game board
    std::vector<cell*> collisions;

    game_board(int n, int m, std::vector<std::vector<cell>> arr);
    void add_tank(tank* t);
    void remove_tank(tank* t);
    void add_shell(shell* s);
    void remove_shell(shell* s);
    void print_board();
    game_board* deep_copy() const;
    string get_board_state();

    bool do_half_step();
    bool do_step();
    void process_shells();
    bool handle_cell_collisions();

    void destroy_all_objects();
};

#endif // BOARD_H