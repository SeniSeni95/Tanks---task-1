#include "GameObject.h"
#include "TankAlgorithm.h"
#include "Board.h"
#include "utils.h"
#include <cmath>
#include <algorithm>
#include <memory>
using namespace std;

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// --------------------
// game_object
// --------------------

game_object::game_object(int x, int y, char symbol)
    : x(x), y(y), symbol(symbol) {}

game_object::game_object()
    : x(0), y(0), symbol(' ') {}

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

// --------------------
// shell
// --------------------

shell::shell(cell* curcell, int directionx, int directiony)
    : directionx(directionx), directiony(directiony), curcell(curcell), just_created(true) {
    x = curcell->get_X();
    y = curcell->get_Y();
    set_shell_symbol();
}

char shell::get_symbol() {
    return '*'; // or symbol if you want to use a field
}
void shell::shell_move_forward(game_board& board) {
    // Remove from current cell
    curcell->remove_Object(this);

    // Move forward
    x = (x + directionx + board.n) % board.n;
    y = (y + directiony + board.m) % board.m;

    curcell = &board.get_cell(x, y);
    curcell->add_Object(board.get_shared_shell(this));

    // If it's just been created, don't trigger collision yet
    if (just_created) {
        just_created = false;
        return;
    }

    // Normal collision detection
    if (curcell->objects.size() > 1 &&
        std::find(board.collisions.begin(), board.collisions.end(), curcell) == board.collisions.end()) {
        for (const auto& ptr : curcell->objects) {
            game_object* obj = ptr.get();
            if ((obj->get_symbol() == '#' || obj->get_symbol() == '1' || obj->get_symbol() == '2' ||
                 (dynamic_cast<shell*>(obj) && obj != this))) {
                board.collisions.push_back(curcell);
                break;
            }
        }
    }
}

void shell::set_shell_symbol() {
    if (directionx == 1 && directiony == 0) shell_symbol = "→";
    else if (directionx == 0 && directiony == -1) shell_symbol = "↑";
    else if (directionx == -1 && directiony == 0) shell_symbol = "←";
    else if (directionx == 0 && directiony == 1) shell_symbol = "↓";
    else if (directionx == 1 && directiony == -1) shell_symbol = "↗";
    else if (directionx == -1 && directiony == -1) shell_symbol = "↖";
    else if (directionx == -1 && directiony == 1) shell_symbol = "↙";
    else if (directionx == 1 && directiony == 1) shell_symbol = "↘";
}

string shell::to_string() {
    return "[ " + shell_symbol + "]";
}

// --------------------
// tank
// --------------------

tank::tank(char symbol, int player_number, int tank_number, int directionx, int directiony, cell* curcell, TankAlgorithm* algo)
    : shells(16),
      player_number(player_number),
      tank_number(tank_number),
      directionx(directionx),
      directiony(directiony),
      shot_timer(0),
      symbol(symbol),
      cannon_symbol(""),
      gear("forward"),
      curcell(curcell),
      algo(algo),
      alive(true)
{
    x = curcell->get_X();
    y = curcell->get_Y();
    set_cannon_symbol();
}
char tank::get_symbol() {
    return symbol; // or whatever field you use for the tank's symbol
}
void tank::move_forward(game_board& board) {
    curcell->remove_Object(this);
    x = (x + directionx + board.n) % board.n;
    y = (y + directiony + board.m) % board.m;
    curcell = &board.get_cell(x, y);
    curcell->add_Object(shared_from_this());

    if (curcell->objects.size() > 1) {
        board.collisions.push_back(curcell);
    }
}

void tank::move_backwards(game_board& board) {
    int new_x = (x - directionx + board.n) % board.n;
    int new_y = (y - directiony + board.m) % board.m;
    cell* newcell = &board.arr[new_x][new_y];

    // Allow moving if the cell is empty or does not have a wall
    if (!newcell->has_Object() || newcell->get_Object()->get_symbol() != 'w') {
        curcell->remove_Object(this);
        curcell = newcell;
        x = new_x;
        y = new_y;
        curcell->add_Object(shared_from_this());
    }

    if (curcell->objects.size() > 1 &&
        std::find(board.collisions.begin(), board.collisions.end(), curcell) == board.collisions.end()) {
        board.collisions.push_back(curcell);
    }
}


void tank::rotate_4(string direction) {
    auto new_dir = ::rotate_4(directionx, directiony, direction);
    directionx = new_dir.first;
    directiony = new_dir.second;
    set_cannon_symbol();
}

void tank::rotate_8(string direction) {
    auto new_dir = ::rotate_8(directionx, directiony, direction);
    directionx = new_dir.first;
    directiony = new_dir.second;
    set_cannon_symbol();
}

void tank::shoot(game_board* board) {
    if (shells > 0) {
        shells--;
        shot_timer = 4;
        cell* curcell = &board->get_cell(x, y);

        auto s = std::make_shared<shell>(curcell, directionx, directiony);
        curcell->add_Object(s);
        board->add_shell(s);  // Add shell to the board's shell list
    }
}


bool tank::turn(game_board* board, const string& move) {
    if (gear == "forward") {
        if (move == "skip") return true;
        if (move == "bw") { gear = "middle"; return true; }
        return handle_move(board, move);
    } else if (gear == "middle") {
        if (move == "fw") { gear = "forward"; return true; }
        gear = "backwards move"; return true;
    } else if (gear == "backwards move") {
        move_backwards(*board);
        gear = "backward";
        return true;
    } else if (gear == "backward") {
        if (move == "skip") return true;
        if (move == "bw") { move_backwards(*board); return true; }
        gear = "forward";
        return handle_move(board, move);
    }

    return false;
}

bool tank::handle_move(game_board* board, const string& move) {
    if (move == "fw") {
        if (wall_coll_check(&board->arr[(x + directionx + board->n) % board->n][(y + directiony + board->m) % board->m]))
            return false;
        move_forward(*board);
        gear = "forward";
        return true;
    } else if (move == "r4l") { rotate_4("left"); gear = "forward"; return true; }
    else if (move == "r8l") { rotate_8("left"); gear = "forward"; return true; }
    else if (move == "r4r") { rotate_4("right"); gear = "forward"; return true; }
    else if (move == "r8r") { rotate_8("right"); gear = "forward"; return true; }
    else if (move == "shoot") {
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
        return dest->get_Object()->get_symbol() == '#';
    }
    return false;
}

void tank::set_cannon_symbol() {
    double degree = atan2(-directiony, directionx) * (180.0 / M_PI);
    if (degree < 0) degree += 360;
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

string tank::to_string() {
    return "[" + (symbol + cannon_symbol) + "]";
}

// --------------------
// mine
// --------------------

mine::mine(char symbol, cell* curcell)
    : curcell(curcell) {
    this->symbol = symbol;
    x = curcell->get_X();
    y = curcell->get_Y();
}

string mine::to_string() {
    return "[ " + std::string(1, symbol) + "]";
}

// --------------------
// wall
// --------------------

wall::wall(char symbol, cell* curcell)
    : hp(2), curcell(curcell) {
    this->symbol = symbol;
    x = curcell->get_X();
    y = curcell->get_Y();
}

string wall::to_string() {
    return "[ " + std::string(1, symbol) + "]";
}
