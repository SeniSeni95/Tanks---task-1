#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <math.h>
#include "utils.h"
#include "board.h"
#include "game_objects.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

const int OUT_OF_BOUNDS = -10000; // Constant for out of bounds score
const int DEATH = -100000;       // Constant for death score
const int WIN = 20; // Constant for win score (not as high as death as it isn't guaranteed)

struct algorithm {
protected:
    std::vector<wall*> walls;
    std::vector<mine*> mines;

public:
    algorithm();
    virtual double base_score(game_board* board_copy, tank* self_copy, int lookahead) = 0;

    void move_other_tanks(game_board* board, tank* self);
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
    std::pair<std::string, double> decide_move(game_board* board, tank* self, int lookahead);
};

struct shell_avoidance_algorithm : public algorithm {
protected:
    int shell_danger_radius;
    int mine_danger_radius;

public:
    shell_avoidance_algorithm();
    virtual double score_position(game_board* board_copy, tank* self_copy);
    virtual double base_score(game_board* board_copy, tank* self_copy, int lookahead) override;
};

double find_shortest_path(Vector2D start, Vector2D end, game_board* board_copy);

struct chasing_algorithm : public shell_avoidance_algorithm {
protected:
    int shell_danger_radius;
    int mine_danger_radius;

public:
    chasing_algorithm();
    virtual double score_position(game_board* board_copy, tank* self_copy) override;
};

#endif // ALGORITHMS_H