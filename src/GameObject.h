#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <memory>

// Forward declarations
struct game_board;
struct cell;
struct TankAlgorithm;

// ========================
// Base Class: game_object
// ========================
struct game_object {
    int x;
    int y;
    char symbol;

    game_object();
    game_object(int x, int y, char symbol);
    virtual ~game_object() = default;

    [[nodiscard]] char get_symbol();
    void set_symbol(char s);
    void set_x(int x);
    void set_y(int y);
    [[nodiscard]] int get_x();
    [[nodiscard]] int get_y();

    virtual std::string to_string() = 0;

    void print() {
        std::cout << to_string();
    }
};

// ==========
// shell
// ==========
struct shell : public game_object {
    int directionx;
    int directiony;
    cell* curcell;
    std::string shell_symbol;
    bool just_created;

    // Constructor does NOT insert into cell
    shell(cell* curcell, int directionx, int directiony);

    void shell_move_forward(game_board& board);
    void set_shell_symbol();
    std::string to_string() override;
};

// ==========
// tank
// ==========
struct tank : public game_object, public std::enable_shared_from_this<tank> {
    int shells;
    int directionx;
    int directiony;
    int shot_timer;
    char symbol;
    std::string cannon_symbol;
    std::string gear;
    cell* curcell;
    TankAlgorithm* algo;  // Not owned
    bool alive;

    // Constructor does NOT insert into cell
    tank(char symbol, int directionx, int directiony, cell* curcell, TankAlgorithm* algo);

    void move_forward(game_board& board);
    void move_backwards(game_board& board);
    void rotate_4(std::string direction);
    void rotate_8(std::string direction);
    void shoot(game_board* board);
    [[nodiscard]] std::string get_cannon_symbol();
    void set_cannon_symbol();
    bool turn(game_board* board, const std::string& move);
    bool handle_move(game_board* board, const std::string& move);
    bool wall_coll_check(cell* dest);
    std::string to_string() override;
};

// ==========
// mine
// ==========
struct mine : public game_object {
    cell* curcell;

    // Constructor does NOT insert into cell
    mine(char symbol, cell* curcell);

    std::string to_string() override;
};

// ==========
// wall
// ==========
struct wall : public game_object {
    int hp;
    cell* curcell;

    // Constructor does NOT insert into cell
    wall(char symbol, cell* curcell);

    std::string to_string() override;
};

#endif // GAMEOBJECT_H
