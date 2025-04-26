#include <iostream>
#include <vector>
#include <string>
#include <math.h>
#include <algorithm>
#include "utils.h"
#include "board.h"
#include "game_objects.h"
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
    game_board* board;
    tank* self;
    std::vector<wall*> walls;
    std::vector<mine*> mines;

public:
    algorithm(game_board* board, tank* self) {
        this->board = board; // Initialize the game board
        this->self = self; // Initialize the self tank
    }

    bool in_bounds(int x, int y) {
        return (x >= 0 && x < board->n && y >= 0 && y < board->m);
    }

    virtual double base_score(int x, int y, int directionx, int directiony, int future) = 0; // score a position
    double base_score(int x, int y, int directionx, int directiony) {
        return base_score(x, y, directionx, directiony, 1); // Call the base score function with future as 0 (== next step)
    }

    virtual double score_forward_move() {
        if (self->gear == "forward" || self->gear.rfind("backward", 0) == 0) {
            int x = self->x + self->directionx;
            int y = self->y + self->directiony;

            if (in_bounds(x, y)) {
                return base_score(x, y, self->directionx, self->directiony);
            }

            return OUT_OF_BOUNDS; // Out of bounds
        } else if (self->gear == "middle") {
            // Move forward will only change gear, not moving yet
            double score_now = base_score(self->x, self->y, self->directionx, self->directiony, 1);

            double x_after = self->x + self->directionx;
            double y_after = self->y + self->directiony;
            double score_after_move = base_score(x_after, y_after, self->directionx, self->directiony, 2);

            if (score_now == DEATH || score_after_move == DEATH) {
                return DEATH;
            }

            // Weighted average of future positions (earlier ones are more important because future is uncertain)
            return score_now * 0.7 + score_after_move * 0.3;
        }
    }

    virtual double score_backward_move() {
        if (self->gear == "forward") {
            // Move backward will only change gear, not moving yet

            int x_after = self->x - self->directionx;
            int y_after = self->y - self->directiony;

            if (!in_bounds(x_after, y_after)) {
                return OUT_OF_BOUNDS; // Out of bounds
            }

            double score_now = base_score(self->x, self->y, -self->directionx, -self->directiony, 1);
            double score_next = base_score(self->x, self->y, -self->directionx, -self->directiony, 2);
            double score_after_move = base_score(x_after, y_after, -self->directionx, -self->directiony, 3);

            if (score_now == DEATH || score_next == DEATH || score_after_move == DEATH) {
                return DEATH;
            }
            
            // Weighted average of future positions (earlier ones are more important because future is uncertain)
            return score_now * 0.5 + score_next * 0.3 + score_after_move * 0.2;
        } else if (self->gear == "middle") {
            // Move backward will only change gear, not moving yet
            return base_score(self->x, self->y, -self->directionx, -self->directiony);
        } else if (self->gear.rfind("backward", 0) == 0) {
            // Move backward will actually move backward (direction is already reversed)
            int x = self->x + self->directionx;
            int y = self->y + self->directiony;
            if (in_bounds(x, y)) {
                return base_score(x, y, self->directionx, self->directiony);
            }
        }

        return OUT_OF_BOUNDS; // Out of bounds
    }

    virtual double score_rotate_left_quarter() {
        std::pair<int, int> new_direction = ::rotate_4(self->directionx, self->directiony, "left");
        return base_score(self->x, self->y, new_direction.first, new_direction.second);
    }

    virtual double score_rotate_right_quarter() {
        std::pair<int, int> new_direction = ::rotate_4(self->directionx, self->directiony, "right");
        return base_score(self->x, self->y, new_direction.first, new_direction.second);
    }

    virtual double score_rotate_left_eighth() {
        std::pair<int, int> new_direction = ::rotate_8(self->directionx, self->directiony, "left");
        return base_score(self->x, self->y, new_direction.first, new_direction.second);
    }

    virtual double score_rotate_right_eighth() {
        std::pair<int, int> new_direction = ::rotate_8(self->directionx, self->directiony, "right");
        return base_score(self->x, self->y, new_direction.first, new_direction.second);
    }

    virtual double score_shoot() = 0; // score the shoot
    virtual double score_skip() = 0; // score the skip move

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
    
    string decide_move() {
        int forward = score_forward_move();
        int backward = score_backward_move();
        int rotate_left_quarter = score_rotate_left_quarter();
        int rotate_right_quarter = score_rotate_right_quarter();
        int rotate_left_eighth = score_rotate_left_eighth();
        int rotate_right_eighth = score_rotate_right_eighth();
        int shoot = score_shoot();
        int skip = score_skip();

        // Find the maximum scored move
        int max_score = max({forward, backward, rotate_left_quarter, rotate_right_quarter, rotate_left_eighth, rotate_right_eighth, shoot, skip});
        
        // Decide the move based on the maximum rating
        if (max_score == forward) return "fw";
        else if (max_score == backward) return "bw";
        else if (max_score == rotate_left_quarter) return "r4l";
        else if (max_score == rotate_right_quarter) return "r4r";
        else if (max_score == rotate_left_eighth) return "r8l";
        else if (max_score == rotate_right_eighth) return "r8r";
        else if (max_score == shoot) return "shoot";
        else return "skip"; // Default to skip if no other move is scored higher
    }
};

struct shell_avoidance_algorithm : public algorithm {
protected:
    int shell_danger_radius = 2; // The radius around shells' trajectories considered dangerous
    int mine_danger_radius = 2; // The radius around mines considered dangerous

public:
    shell_avoidance_algorithm(game_board* board, tank* self) : algorithm(board, self) {}

    game_board* get_future_board(int future) {
        game_board* future_board = board->deep_copy();
        for (int _ = 0; _ < future; _++) {
            future_board->do_step();
        }
        return future_board;
    }

    /**
     * Scores the position for the next step based on the following parameters:
     * - Distance from shells which are heading towards a ±shell_danger_radius area around the tank.
     * - Distance from mines.
     */
    double score_position(int x, int y, int future) {
        double score = 0;

        // Get future board state
        game_board* future_board = get_future_board(future);
        fetch_walls_and_mines(future_board);

        // Score based on shells:
        for (shell* s : future_board->shells) {
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

    double base_score(int x, int y, int directionx, int directiony, int future) override {
        // Check if the position is out of bounds
        if (!in_bounds(x, y)) {
            return OUT_OF_BOUNDS; // Out of bounds
        }

        // Score the position based on distance from shells and mines
        double score = score_position(x, y, future);

        // Add additional scoring logic here if needed

        return score;
    }

    double score_forward_move() override {
        // Implement logic to score the forward move
        return 0; // Placeholder
    }

    double score_backward_move() override {
        // Implement logic to score the backward move
        return 0; // Placeholder
    }

    double score_rotate_left_quarter() override {
        // Implement logic to score the left rotation
        return 0; // Placeholder
    }

    double score_rotate_right_quarter() override {
        // Implement logic to score the right rotation
        return 0; // Placeholder
    }

    double score_rotate_left_eighth() override {
        // Implement logic to score the left rotation
        return 0; // Placeholder
    }

    double score_rotate_right_eighth() override {
        // Implement logic to score the right rotation
        return 0; // Placeholder
    }

    double score_shoot() override {
        // Implement logic to score the shoot action
        return 0; // Placeholder
    }

    double score_skip() override {
        // Implement logic to score the skip action
        return 0; // Placeholder
    }
}