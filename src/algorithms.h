#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <unordered_map>
#include <ctime>
#include <math.h>
#include "utils.h"
#include "Board.h"
#include "GameObject.h"
using namespace std;

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

const int DEATH = -100000;       // Constant for death score
const int WIN = 10; // Constant for win score (not as high as death because it isn't guaranteed)

const int SHELL_DANGER_RADIUS = 2; // Shell danger radius (distance from the trajectory line)
const int SHELL_DANGER_DISTANCE = 4; // Shell danger distance
const int MINE_DANGER_RADIUS = 2;  // Mine danger radius

// Forward declarations
class game_board;
class tank;
class shell;
class mine;
class wall;
class cell;
class Vector2D;

class algorithm {
protected:
    std::vector<wall*> walls;
    std::vector<mine*> mines;
    unordered_map<std::string, time_t> board_states;

public:
    algorithm();

    virtual ~algorithm() = default;

    virtual double base_score(game_board* board_copy, tank* self_copy, int lookahead) = 0;

    void other_tanks_turn(game_board* board, tank* self);
    void do_move(game_board* board, tank* self, const std::string& move);
    double score_single_move(game_board* board, tank* self, const std::string& move, int lookahead);
    virtual double score_forward_move(game_board* board, tank* self, int lookahead);
    virtual double score_backward_move(game_board* board, tank* self, int lookahead);
    virtual double score_rotate_left_quarter(game_board* board, tank* self, int lookahead);
    virtual double score_rotate_right_quarter(game_board* board, tank* self, int lookahead);
    virtual double score_rotate_left_eighth(game_board* board, tank* self, int lookahead);
    virtual double score_rotate_right_eighth(game_board* board, tank* self, int lookahead);
    virtual double score_shoot(game_board* board, tank* self, int lookahead);
    virtual double score_skip(game_board* board, tank* self, int lookahead);
    tank* get_self_in_board_copy(game_board* board_copy, tank* self);
    void fetch_walls_and_mines(game_board* board);
    std::pair<std::string, double> decide_move(game_board* board, tank* self, int lookahead, bool first_call = true);
};

class shell_avoidance_algorithm : public algorithm {
protected:
    int shell_danger_radius;
    int shell_danger_distance;
    int mine_danger_radius;

public:
    shell_avoidance_algorithm();
    virtual double score_position(game_board* board_copy, tank* self_copy);
    virtual double base_score(game_board* board_copy, tank* self_copy, int lookahead) override;
};

int find_shortest_path(Vector2D start, Vector2D end, game_board* board_copy);

class chasing_algorithm : public shell_avoidance_algorithm {
public:
    chasing_algorithm();
    virtual double score_position(game_board* board_copy, tank* self_copy) override;
    virtual double score_shoot(game_board* board, tank* self, int lookahead) override;
};

class running_algorithm : public shell_avoidance_algorithm {
public:
    running_algorithm();
    virtual double score_position(game_board* board_copy, tank* self_copy) override;
};

#endif // ALGORITHMS_H