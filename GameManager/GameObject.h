#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <memory>

// Forward declarations
class game_board;
class cell;
class TankAlgorithm;

// ========================
// Base Class: game_object
// ========================
class game_object {
protected:
    int x;
    int y;
    char symbol;

public:
    game_object();
    game_object(int x, int y, char symbol);
    virtual char get_symbol();
    virtual ~game_object() = default;

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
class shell : public game_object {
public:
    int directionx;
    int directiony;
    cell* curcell;
    std::string shell_symbol;
    bool just_created;


    shell(cell* curcell, int directionx, int directiony);
    char get_symbol() override;
    void shell_move_forward(game_board& board);
    void set_shell_symbol();
    std::string to_string() override;
};

// ==========
// tank
// ==========
class tank : public game_object, public std::enable_shared_from_this<tank> {
public:
    int shells;
    int player_number; // 0 or 1
    int tank_number;   // per-player index
    int directionx;
    int directiony;
    int shot_timer;
    char symbol;
    std::string cannon_symbol;
    std::string gear;
    cell* curcell;
    TankAlgorithm* algo;  // Not owned
    bool alive;

    tank(char symbol, int player_number, int tank_number, int directionx, int directiony, cell* curcell, TankAlgorithm* algo);
    char get_symbol() override;
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
class mine : public game_object {
public:
    cell* curcell;

    mine(char symbol, cell* curcell);
    std::string to_string() override;
};

// ==========
// wall
// ==========
class wall : public game_object {
public:
    int hp;
    cell* curcell;

    wall(char symbol, cell* curcell);
    std::string to_string() override;
};

#endif // GAMEOBJECT_H
