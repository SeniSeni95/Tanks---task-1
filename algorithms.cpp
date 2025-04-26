#include <iostream>
#include <vector>
#include <string>
#include <math.h>
#include <algorithm>
#include "utils.h"
#include "board.h"
#include "game_objects.h"
#include "algorithms.h"
using namespace std;
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Forward declarations
struct game_object;
struct tank;
struct shell;
struct mine;
struct wall;
struct game_board;
struct cell;
struct game_master;
struct algorithm;

const int OUT_OF_BOUNDS = -10000; // Constant for out of bounds score
const int DEATH = -100000; // Constant for death score

// An algorithm is responsible to make one move per turn, given the entire game state
struct algorithm {
protected:
    std::vector<wall*> walls;
    std::vector<mine*> mines;

public:
    algorithm() {}

    virtual double base_score(game_board* board_copy, tank* self_copy) = 0; // score a tank in a board

    void move_other_tanks(game_board* board, tank* self) {
        for (tank* t : board->tanks) {
            if (t->symbol != self->symbol) {
                t->handle_move(board, "fw");
            }
        }
    }

    void do_move(game_board* board, tank* self, const string& move) {
        self->handle_move(board, move);
        move_other_tanks(board, self);
        board->do_step();
    }

    double score_single_move(game_board* board, tank* self, const string& move) {
        game_board* board_copy = board->deep_copy();
        tank* self_copy = get_self_in_board_copy(board_copy, self);

        do_move(board_copy, self_copy, move);
        return base_score(board_copy, self_copy);
    }

    virtual double score_forward_move(game_board* board, tank* self) {
        game_board* board_copy = board->deep_copy();
        tank* self_copy = get_self_in_board_copy(board_copy, self);

        do_move(board_copy, self_copy, "fw");

        return base_score(board_copy, self_copy);
    }

    virtual double score_backward_move(game_board* board, tank* self) {
        game_board* board_copy = board->deep_copy();
        tank* self_copy = get_self_in_board_copy(board_copy, self);

        if (self->gear == "forward") {
            // Move backward will only change gear, not moving yet

            // Not moving yet
            do_move(board_copy, self_copy, "bw");
            double score_now = base_score(board_copy, self_copy);

            // Not moving yet
            do_move(board_copy, self_copy, "bw");
            double score_next = base_score(board_copy, self_copy);

            // Actually moving now
            do_move(board_copy, self_copy, "bw");
            double score_after_move = base_score(board_copy, self_copy);

            if (score_now == DEATH || score_next == DEATH || score_after_move == DEATH) {
                return DEATH;
            }
            
            // Weighted average of future positions (earlier ones are more important because future is uncertain)
            return score_now * 0.5 + score_next * 0.3 + score_after_move * 0.2;
        } else if (self->gear == "middle") {
            // Not moving yet
            do_move(board_copy, self_copy, "bw");
            double score_now = base_score(board_copy, self_copy);

            // Actually moving now
            do_move(board_copy, self_copy, "bw");
            double score_after_move = base_score(board_copy, self_copy);

            if (score_now == DEATH || score_after_move == DEATH) {
                return DEATH;
            }
            
            // Weighted average of future positions (earlier ones are more important because future is uncertain)
            return score_now * 0.7 + score_after_move * 0.3;
        } else {
            do_move(board_copy, self_copy, "bw");
            return base_score(board_copy, self_copy);
        }
    }

    virtual double score_rotate_left_quarter(game_board* board, tank* self) {
        return score_single_move(board, self, "r4l");
    }

    virtual double score_rotate_right_quarter(game_board* board, tank* self) {
        return score_single_move(board, self, "r4r");
    }

    virtual double score_rotate_left_eighth(game_board* board, tank* self) {
        return score_single_move(board, self, "r8l");
    }

    virtual double score_rotate_right_eighth(game_board* board, tank* self) {
        return score_single_move(board, self, "r8r");
    }

    virtual double score_shoot(game_board* board, tank* self) {
        return score_single_move(board, self, "shoot");
    }

    virtual double score_skip(game_board* board, tank* self) {
        return score_single_move(board, self, "skip");
    }

    tank* get_self_in_board_copy(game_board* board_copy, tank* self) {
        for (tank* t : board_copy->tanks) {
            if (t->symbol == self->symbol) {
                return t;
            }
        }
        return nullptr; // Tank not found
    }

    void fetch_walls_and_mines(game_board* board) {
        walls.clear();
        mines.clear();
        for (int i = 0; i < board->n; i++) {
            for (int j = 0; j < board->m; j++) {
                cell* c = &board->arr[i][j];
                if (c->has_Object()) {
                    if (c->get_Object()->get_symbol() == 'w') {
                        walls.push_back(dynamic_cast<wall*>(c->get_Object()));
                    } else if (c->get_Object()->get_symbol() == 'm') {
                        mines.push_back(dynamic_cast<mine*>(c->get_Object()));
                    }
                }
            }
        }
    }

    std::pair<string, double> decide_move(game_board* board, tank* self) {
        double forward = score_forward_move(board, self);
        double backward = score_backward_move(board, self);
        double rotate_left_quarter = score_rotate_left_quarter(board, self);
        double rotate_right_quarter = score_rotate_right_quarter(board, self);
        double rotate_left_eighth = score_rotate_left_eighth(board, self);
        double rotate_right_eighth = score_rotate_right_eighth(board, self);
        double shoot = score_shoot(board, self);
        double skip = score_skip(board, self);

        // Find the maximum scored move
        double max_score = max({forward, backward, rotate_left_quarter, rotate_right_quarter, rotate_left_eighth, rotate_right_eighth, shoot, skip});
        
        // Decide the move based on the maximum rating
        if (max_score == forward) return {"fw", max_score};
        else if (max_score == backward) return {"bw", max_score};
        else if (max_score == rotate_left_quarter) return {"r4l", max_score};
        else if (max_score == rotate_right_quarter) return {"r4r", max_score};
        else if (max_score == rotate_left_eighth) return {"r8l", max_score};
        else if (max_score == rotate_right_eighth) return {"r8r", max_score};
        else if (max_score == shoot) return {"shoot", max_score};
        else return {"skip", max_score}; // Default to skip if no other move is scored higher
    }
};

struct shell_avoidance_algorithm : public algorithm {
protected:
    int shell_danger_radius = 2; // The radius around shells' trajectories considered dangerous
    int mine_danger_radius = 2; // The radius around mines considered dangerous

public:
    shell_avoidance_algorithm() : algorithm() {}

    /**
     * Scores the position for the next step based on the following parameters:
     * - Distance from shells which are heading towards a ±shell_danger_radius area around the tank.
     * - Distance from mines.
     */
    double score_position(int x, int y, game_board* board_copy) {
        double score = 0;

        // Score based on shells:
        for (shell* s : board_copy->shells) {
            Vector2D shell_pos = {s->x, s->y};
            Vector2D shell_dir = {s->directionx, s->directiony};

            Vector2D tank_pos = {x, y};

            if (shell_pos.x == tank_pos.x && shell_pos.y == tank_pos.y) {
                return DEATH; // Direct hit, avoid at all costs
            }

            // Calculate closest point between the shell's trajectory and the tank's position
            double dist_to_traj = chebyshevDistanceToLine(shell_pos, shell_dir, tank_pos);
            if (dist_to_traj <= shell_danger_radius) {
                // Shell is within danger radius, score negatively (proportional to distance)
                double distance_to_shell = tank_pos.chebyshevDistance(shell_pos);
                score -= (shell_danger_radius + 1 - dist_to_traj) / distance_to_shell; // Penalize based on distance
            }
        }

        // Score based on distance to mines:
        for (mine* m : mines) {
            Vector2D mine_pos = {m->x, m->y};
            Vector2D tank_pos = {x, y};

            double distance_to_mine = tank_pos.chebyshevDistance(mine_pos);

            if (distance_to_mine == 0) {
                return DEATH; // Direct hit, avoid at all costs
            }

            if (distance_to_mine <= mine_danger_radius) {
                // Mine is within danger radius, score negatively
                score -= (mine_danger_radius + 1 - distance_to_mine); // Penalize based on distance
            }
        }

        return score;
    }

    double base_score(game_board* board_copy, tank* self_copy) override {
        fetch_walls_and_mines(board_copy);

        // Score the position
        double score = score_position(self_copy->x, self_copy->y, board_copy);

        // Also calculate the next best possible move
        std::pair<string, double> best_move = decide_move(board_copy, self_copy);

        return score;
    }
};
