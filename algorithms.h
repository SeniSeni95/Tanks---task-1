#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include "utils.h"
#include "board.h"
#include "game_objects.h"

const int OUT_OF_BOUNDS = -10000; // Constant for out of bounds score
const int DEATH = -100000;       // Constant for death score

struct algorithm {
protected:
    std::vector<wall*> walls;
    std::vector<mine*> mines;

public:
    algorithm();
    virtual double base_score(game_board* board_copy, tank* self_copy) = 0;

    void move_other_tanks(game_board* board, tank* self);
    void do_move(game_board* board, tank* self, const std::string& move);
    double score_single_move(game_board* board, tank* self, const std::string& move);
    virtual double score_forward_move(game_board* board, tank* self);
    virtual double score_backward_move(game_board* board, tank* self);
    virtual double score_rotate_left_quarter(game_board* board, tank* self);
    virtual double score_rotate_right_quarter(game_board* board, tank* self);
    virtual double score_rotate_left_eighth(game_board* board, tank* self);
    virtual double score_rotate_right_eighth(game_board* board, tank* self);
    virtual double score_shoot(game_board* board, tank* self);
    virtual double score_skip(game_board* board, tank* self);
    tank* get_self_in_board_copy(game_board* board_copy, tank* self);
    void fetch_walls_and_mines(game_board* board);
    std::pair<std::string, double> decide_move(game_board* board, tank* self);
};

struct shell_avoidance_algorithm : public algorithm {
protected:
    int shell_danger_radius;
    int mine_danger_radius;

public:
    shell_avoidance_algorithm();
    double score_position(int x, int y, game_board* board_copy);
    double base_score(game_board* board_copy, tank* self_copy) override;
};

#endif // ALGORITHMS_H