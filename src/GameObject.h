#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include <iostream>
#include <vector>
#include <string>
#include <math.h>
#include "TankAlgorithm.h"
#include "board.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace std;

// Forward declarations
struct game_board;
struct cell;
struct tank;
struct shell;
struct mine;
struct wall;
struct algorithm;

struct game_object {
    int x;
    int y;
    char symbol;

    game_object(int x, int y, char symbol);
    game_object();

    // Destructor
    virtual ~game_object() = default;

    char get_symbol();
    void set_symbol(char s);
    void set_x(int x);
    void set_y(int y);
    int get_x();
    int get_y();
    virtual string to_string() = 0; // Pure virtual function for polymorphism

    void print() {
        string str = to_string();
        cout << str;
    }
};

struct shell : public game_object {
    int directionx;
    int directiony;
    cell* curcell;
    string shell_symbol;
    bool just_created;

    shell(cell* curcell, int directionx, int directiony);
    void shell_move_forward(game_board& board);
    void set_shell_symbol();
    string to_string();
};

struct tank : public game_object {
    int shells;
    int directionx;
    int directiony;
    int shot_timer;
    string cannon_symbol;
    string gear;
    cell* curcell;
    TankAlgorithm* algo;
    bool alive;

    tank(char symbol, int directionx, int directiony, cell* curcell, TankAlgorithm* algo);
    void move_forward(game_board& board);
    void move_backwards(game_board& board);
    void rotate_4(string direction);
    void rotate_8(string direction);
    void shoot(game_board* board);
    string get_cannon_symbol();
    void set_cannon_symbol();
    bool turn(game_board* board, const string& move);
    bool handle_move(game_board* board, const string& move);
    bool wall_coll_check(cell* dest);
    string to_string();
};

struct mine : public game_object {
    cell* curcell;

    mine(char symbol, cell* curcell);
    string to_string();
};

struct wall : public game_object {
    int hp;
    cell* curcell;

    wall(char symbol, cell* curcell);
    string to_string();
};

#endif // GAMEOBJECT_H