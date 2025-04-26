#include "game_objects.h"
#include "board.h"
#include "utils.h"
#include <math.h>
#include <algorithm>
using namespace std;

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

game_object::game_object(int x, int y, char symbol) {
    this->x = x;
    this->y = y;
    this->symbol = symbol;
}

game_object::game_object() {
    this->x = 0;
    this->y = 0;
    this->symbol = ' ';
}

char game_object::get_symbol() {
    return symbol;
}

void game_object::set_symbol(char s) {
    symbol = s;
}

void game_object::set_x(int x) {
    this->x = x;
}

void game_object::set_y(int y) {
    this->y = y;
}

int game_object::get_x() {
    return x;
}

int game_object::get_y() {
    return y;
}

shell::shell(cell* curcell, int directionx, int directiony) {
    this->curcell = curcell;
    this->x = curcell->get_X();
    this->y = curcell->get_Y();
    this->directionx = directionx;
    this->directiony = directiony;
    this->set_shell_symbol();
    curcell->add_Object(this);
}

void shell::shell_move_forward(game_board& board) {
    curcell->remove_Object(this);

    x = (x + directionx + board.n) % board.n;
    y = (y + directiony + board.m) % board.m;

    curcell = &board.arr[x][y];
    curcell->add_Object(this);

    if (curcell->objects.size() > 1 &&
        find(board.collisions.begin(), board.collisions.end(), curcell) == board.collisions.end()) {
        bool has_mine = false;
        for (game_object* obj : curcell->objects) {
            if (obj->symbol == 'm') {
                has_mine = true;
                break;
            }
        }

        if (!has_mine) {
            board.collisions.push_back(curcell);
        }
    }
}

void shell::set_shell_symbol() {
    double degree = atan2(-directionx, directiony) * (180.0 / M_PI);
    if (degree < 0) {
        degree += 360;
    }
    if (degree == 0) shell_symbol = "→";
    else if (degree == 90) shell_symbol = "↑";
    else if (degree == 180) shell_symbol = "←";
    else if (degree == 270) shell_symbol = "↓";
    else if (degree == 45) shell_symbol = "↗";
    else if (degree == 135) shell_symbol = "↖";
    else if (degree == 225) shell_symbol = "↙";
    else if (degree == 315) shell_symbol = "↘";
}

void shell::print() {
    cout << "[ " << shell_symbol << "]";
}

tank::tank(char symbol, int directiony, int directionx, cell* curcell) {
    this->curcell = curcell;
    curcell->add_Object(this);
    this->x = curcell->get_X();
    this->y = curcell->get_Y();
    this->symbol = symbol;
    this->directionx = directiony;
    this->directiony = directionx;
    set_cannon_symbol();
    this->shells = 16;
    this->shot_timer = 0;
    this->gear = "forward";
}

void tank::move_forward(game_board& board) {
    curcell->remove_Object(this);
    x = (x + directionx + board.n) % board.n;
    y = (y + directiony + board.m) % board.m;
    curcell = &board.arr[x][y];
    curcell->add_Object(this);

    if (curcell->objects.size() > 1) {
        board.collisions.push_back(curcell);
    }
}

void tank::move_backwards(game_board& board) {
    curcell->remove_Object(this);
    x = (x - directionx + board.n) % board.n;
    y = (y - directiony + board.m) % board.m;
    curcell = &board.arr[x][y];
    curcell->add_Object(this);

    if (curcell->objects.size() > 1 &&
        find(board.collisions.begin(), board.collisions.end(), curcell) == board.collisions.end()) {
        board.collisions.push_back(curcell);
    }
}

void tank::rotate_4(string direction) {
    std::pair<int, int> new_direction = ::rotate_4(directionx, directiony, direction);
    directionx = new_direction.first;
    directiony = new_direction.second;
    set_cannon_symbol();
}

void tank::rotate_8(string direction) {
    std::pair<int, int> new_direction = ::rotate_8(directionx, directiony, direction);
    directionx = new_direction.first;
    directiony = new_direction.second;
    set_cannon_symbol();
}

void tank::shoot(game_board* board) {
    if (shells > 0) {
        shells--;
        shot_timer = 4;
        shell* s = new shell(curcell, directionx, directiony);
        board->add_shell(s);
    }
}

bool tank::turn(game_board* board, const string& move) {
    // Handle gear logic
    if (gear == "forward") {
        if (move == "skip") {
            return true;
        } else if (move == "bw") {
            gear = "middle"; // Change gear to middle
            return true;
        } else {
            // Handle other moves (forward, rotate, shoot, etc.)
            return handle_move(board, move);
        }
    } else if (gear == "middle") {
        if (move == "fw") {
            gear = "forward"; // Change gear to forward
            return true;
        } else {
            gear = "backwards move"; // Change gear to backwards move
            return true;
        }
    } else if (gear == "backwards move") {
        // Automatically move backwards without asking for input
        move_backwards(*board);
        gear = "backward"; // Change gear to backward
        return true;
    } else if (gear == "backward") {
        if (move == "skip") {
            return true;
        } else if (move == "bw") {
            move_backwards(*board); // Move backwards
            return true;
        } else {
            // Handle other moves (forward, rotate, shoot, etc.)
            gear = "forward"; // Change gear to forward
            return handle_move(board, move);
        }
    }

    return false; // Return false if no valid tank was found
}

bool tank::handle_move(game_board* board, const string& move) {
    if (move == "fw") {
        if (wall_coll_check(&board->arr[(x + directionx + board->n) % board->n][(y + directiony + board->m) % board->m])) {
            return false;
        } else {
            move_forward(*board);
            gear = "forward";
            return true;
        }
    } else if (move == "r4l") {
        rotate_4("left");
        gear = "forward";
        return true;
    } else if (move == "r8l") {
        rotate_8("left");
        gear = "forward";
        return true;
    } else if (move == "r4r") {
        rotate_4("right");
        gear = "forward";
        return true;
    } else if (move == "r8r") {
        rotate_8("right");
        gear = "forward";
        return true;
    } else if (move == "shoot") {
        if (shot_timer == 0) {
            shoot(board);
            gear = "forward";
            return true;
        }
        return false;
    }
    return false;
}

bool tank::wall_coll_check(cell* dest) {
    if (dest->has_Object()) {
        game_object* obj = dest->get_Object();
        if (obj->symbol == 'w') {
            return true;
        }
    }
    return false;
}

void tank::set_cannon_symbol() {
    double degree = atan2(-directionx, directiony) * (180.0 / M_PI);
    if (degree < 0) {
        degree += 360;
    }
    if (degree == 0) cannon_symbol = "→";
    else if (degree == 90) cannon_symbol = "↑";
    else if (degree == 180) cannon_symbol = "←";
    else if (degree == 270) cannon_symbol = "↓";
    else if (degree == 45) cannon_symbol = "↗";
    else if (degree == 135) cannon_symbol = "↖";
    else if (degree == 225) cannon_symbol = "↙";
    else if (degree == 315) cannon_symbol = "↘";
}

string tank::get_cannon_symbol() {
    return cannon_symbol;
}

void tank::print() {
    cout << "[" << symbol << cannon_symbol << "]";
}

mine::mine(char symbol, cell* curcell) {
    this->curcell = curcell;
    this->symbol = symbol;
    this->x = curcell->get_X();
    this->y = curcell->get_Y();
    curcell->add_Object(this);
}

void mine::print() {
    cout << "[ " << symbol << "]";
}

wall::wall(char symbol, cell* curcell) {
    this->curcell = curcell;
    this->symbol = symbol;
    this->x = curcell->get_X();
    this->y = curcell->get_Y();
    curcell->add_Object(this);
    this->hp = 2;
}

void wall::print() {
    cout << "[ " << symbol << "]";
}