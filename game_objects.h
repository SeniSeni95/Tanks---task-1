#ifndef GAME_OBJECTS_H
#define GAME_OBJECTS_H

#include <iostream>
#include <vector>
#include <string>
#include <math.h>
#include <algorithm>
#include "board.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace std;

struct game_object {
    int x;
    int y;
    char symbol;

    game_object(int x, int y, char symbol);
    game_object();
    char get_symbol();
    void set_symbol(char s);
    void set_x(int x);
    void set_y(int y);
    int get_x();
    int get_y();
    virtual void print() = 0; // Pure virtual function for polymorphism
};

struct shell : public game_object {
    int directionx;
    int directiony;
    cell* curcell;
    string shell_symbol;

    shell(cell* curcell, int directionx, int directiony);
    void shell_move_forward(game_board& board);
    void set_shell_symbol();
    void print();
};

struct tank : public game_object {
    int shells;
    int directionx;
    int directiony;
    int shot_timer;
    string cannon_symbol;
    string gear;
    cell* curcell;

    tank(char symbol, int directiony, int directionx, cell* curcell);
    void move_forward(game_board& board);
    void move_backwards(game_board& board);
    void rotate_4(string direction);
    void rotate_8(string direction);
    void shoot(game_board* board);
    string get_cannon_symbol();
    void set_cannon_symbol();
    bool handle_move(game_board* board, const string& move);
    void print();
};

struct mine : public game_object {
    cell* curcell;

    mine(char symbol, cell* curcell);
    void print();
};

struct wall : public game_object {
    int hp;
    cell* curcell;

    wall(char symbol, cell* curcell);
    void print();
};

#endif // GAME_OBJECTS_H