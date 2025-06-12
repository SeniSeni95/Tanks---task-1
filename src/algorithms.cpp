#include "algorithms.h"
#include <queue>

algorithm::algorithm() {}

void algorithm::other_tanks_turn(game_board* board, shared_ptr<tank> self) {
    for (auto& t : board->tanks) {
        if (t->symbol != self->symbol) {
            t->turn(board, "shoot"); // Assume everyone else tries to kill us
        }
    }
}

void algorithm::do_move(game_board* board, shared_ptr<tank> self, const std::string& move) {    
    board->simulate_step(std::make_tuple(
        self->get_x(),
        self->get_y(),
        move
    ));
}

double algorithm::score_single_move(game_board* board, shared_ptr<tank> self, const std::string& move, int lookahead, int stepsSinceBoardUpdate) {
    unique_ptr<game_board> board_copy = board->dummy_copy();
    shared_ptr<tank> self_copy = get_self_in_board_copy(board_copy.get(), self);

    do_move(board_copy.get(), self_copy, move);
    double score = base_score(board_copy.get(), self_copy, lookahead, stepsSinceBoardUpdate);

    return score;
}

double algorithm::score_forward_move(game_board* board, shared_ptr<tank> self, int lookahead, int stepsSinceBoardUpdate) {
    return score_single_move(board, self, "fw", lookahead, stepsSinceBoardUpdate);
}

double algorithm::score_backward_move(game_board* board, shared_ptr<tank> self, int lookahead, int stepsSinceBoardUpdate) {
    return score_single_move(board, self, "bw", lookahead, stepsSinceBoardUpdate);
}

double algorithm::score_rotate_left_quarter(game_board* board, shared_ptr<tank> self, int lookahead, int stepsSinceBoardUpdate) {
    return score_single_move(board, self, "r4l", lookahead, stepsSinceBoardUpdate);
}

double algorithm::score_rotate_right_quarter(game_board* board, shared_ptr<tank> self, int lookahead, int stepsSinceBoardUpdate) {
    return score_single_move(board, self, "r4r", lookahead, stepsSinceBoardUpdate);
}

double algorithm::score_rotate_left_eighth(game_board* board, shared_ptr<tank> self, int lookahead, int stepsSinceBoardUpdate) {
    return score_single_move(board, self, "r8l", lookahead, stepsSinceBoardUpdate);
}

double algorithm::score_rotate_right_eighth(game_board* board, shared_ptr<tank> self, int lookahead, int stepsSinceBoardUpdate) {
    return score_single_move(board, self, "r8r", lookahead, stepsSinceBoardUpdate);
}

double algorithm::score_shoot(game_board* board, shared_ptr<tank> self, int lookahead, int stepsSinceBoardUpdate) {
    // Check if the tank can shoot
    if (self->shot_timer > 0) {
        return -100000; // Cannot shoot
    }
    return score_single_move(board, self, "shoot", lookahead, stepsSinceBoardUpdate);
}

double algorithm::score_skip(game_board* board, shared_ptr<tank> self, int lookahead, int stepsSinceBoardUpdate) {
    return score_single_move(board, self, "skip", lookahead, stepsSinceBoardUpdate);
}

double algorithm::score_update_board(game_board* board, shared_ptr<tank> self, int lookahead) {
    return score_single_move(board, self, "update", lookahead, 0); // Set stepsSinceBoardUpdate to 0 for board update
}

shared_ptr<tank> algorithm::get_self_in_board_copy(game_board* board_copy, shared_ptr<tank> self) {
    for (auto& t : board_copy->tanks) {
        if (t->player_number == self->player_number && t->tank_number == self->tank_number) {
            return t;
        }
    }
    std::cout << "Error: Tank not found in board copy!" << std::endl;
    return nullptr;
}

void algorithm::fetch_walls_and_mines(game_board* board) {
    walls.clear();
    mines.clear();
    for (int i = 0; i < board->n; i++) {
        for (int j = 0; j < board->m; j++) {
            cell* c = &board->get_cell(i, j);
            if (c->has_Object()) {
                if (c->get_Object()->get_symbol() == '#') {
                    walls.push_back(dynamic_cast<wall*>(c->get_Object()));
                } else if (c->get_Object()->get_symbol() == '@') {
                    mines.push_back(dynamic_cast<mine*>(c->get_Object()));
                }
            }
        }
    }
}

std::pair<std::string, double> algorithm::decide_move(game_board* board, shared_ptr<tank> self, int lookahead, int stepsSinceBoardUpdate, bool first_call) {
    string state = board->get_board_state();

    double forward = score_forward_move(board, self, lookahead, stepsSinceBoardUpdate);
    double backward = score_backward_move(board, self, lookahead, stepsSinceBoardUpdate);
    double rotate_left_quarter = score_rotate_left_quarter(board, self, lookahead, stepsSinceBoardUpdate);
    double rotate_right_quarter = score_rotate_right_quarter(board, self, lookahead, stepsSinceBoardUpdate);
    double rotate_left_eighth = score_rotate_left_eighth(board, self, lookahead, stepsSinceBoardUpdate);
    double rotate_right_eighth = score_rotate_right_eighth(board, self, lookahead, stepsSinceBoardUpdate);
    double shoot = score_shoot(board, self, lookahead, stepsSinceBoardUpdate);
    double skip = score_skip(board, self, lookahead, stepsSinceBoardUpdate);
    double update_board = score_update_board(board, self, lookahead);

    if (first_call) {
        cout << "\nScores for tank " << self->symbol << " at (" << self->get_x() << ", " << self->get_y() << "):" << endl;
        cout << "Forward: " << forward << endl;
        cout << "Backward: " << backward << endl;
        cout << "Rotate left quarter: " << rotate_left_quarter << endl;
        cout << "Rotate right quarter: " << rotate_right_quarter << endl;
        cout << "Rotate left eighth: " << rotate_left_eighth << endl;
        cout << "Rotate right eighth: " << rotate_right_eighth << endl;
        cout << "Shoot: " << shoot << endl;
        cout << "Skip: " << skip << endl;
        cout << "Update board: " << update_board << endl;
    }

    vector<double> scores = {forward, backward, rotate_left_quarter, rotate_right_quarter, rotate_left_eighth, rotate_right_eighth, shoot, skip};
    double max_score = *std::max_element(scores.begin(), scores.end());

    if (max_score == forward) return {"fw", max_score};
    else if (max_score == backward) return {"bw", max_score};
    else if (max_score == rotate_left_quarter) return {"r4l", max_score};
    else if (max_score == rotate_right_quarter) return {"r4r", max_score};
    else if (max_score == rotate_left_eighth) return {"r8l", max_score};
    else if (max_score == rotate_right_eighth) return {"r8r", max_score};
    else if (max_score == shoot) return {"shoot", max_score};
    else if (max_score == update_board) return {"update", max_score};
    else return {"skip", max_score};
}

shell_avoidance_algorithm::shell_avoidance_algorithm() : algorithm(), shell_danger_radius(SHELL_DANGER_RADIUS), shell_danger_distance(SHELL_DANGER_DISTANCE), mine_danger_radius(MINE_DANGER_RADIUS) {}

double shell_avoidance_algorithm::score_position(game_board* board_copy, shared_ptr<tank> self_copy) {
    double score = 0;

    for (auto& s : board_copy->shells) {
        Vector2D shell_pos = {s->get_x(), s->get_y()};
        Vector2D shell_dir = {s->directionx, s->directiony};
        Vector2D tank_pos = {self_copy->get_x(), self_copy->get_y()};

        if (shell_pos.x == tank_pos.x && shell_pos.y == tank_pos.y) {
            return DEATH;
        }

        // Returns pair of distance from the trajectory line and actual distance to the shell
        std::pair<int, int> dists = chebyshevDistanceToLine(shell_pos, shell_dir, tank_pos, board_copy);

        int distance_to_trajectory = dists.first;
        int distance_to_shell = dists.second;

        if (distance_to_trajectory <= shell_danger_radius && distance_to_shell <= shell_danger_distance) {
            // cout << "Distance to trajectory: " << distance_to_trajectory << ", Distance to shell: " << distance_to_shell << endl;
            score -= pow(20.0 / (double) (distance_to_trajectory + 1), 2); // Avoid being on the trajectory line at all costs
            score -= 20.0 / (double) (distance_to_shell + 1); // Also try getting away from the shell, but not as important
        }
    }

    for (mine* m : mines) {
        Vector2D mine_pos = {m->get_x(),m->get_y()};
        Vector2D tank_pos = {self_copy->get_x(), self_copy->get_y()};

        double distance_to_mine = tank_pos.chebyshevDistance(mine_pos);

        if (distance_to_mine == 0) {
            return DEATH;
        }

        if (distance_to_mine <= mine_danger_radius) {
            score -= (mine_danger_radius + 1 - distance_to_mine);
        }
    }

    return score;
}

double shell_avoidance_algorithm::base_score(game_board* board_copy, shared_ptr<tank> self_copy, int lookahead, int stepsSinceBoardUpdate) {
    // First, check if the tank still exists in the board copy
    bool tank_exists = false;
    for (auto& t : board_copy->tanks) {
        if (t->player_number == self_copy->player_number && t->tank_number == self_copy->tank_number && t->alive) {
            tank_exists = true;
            break;
        }
    }

    double score = 0;

    if (!tank_exists) {
        return DEATH; // Tank does not exist in the board copy
    } else {
        // Check how many enemy/ally tanks are alive
        int enemy_tanks_alive = 0;
        int ally_tanks_alive = 0;
        for (auto& t : board_copy->tanks) {
            if (t->alive) {
                if (t->player_number == self_copy->player_number) {
                    ally_tanks_alive++;
                } else {
                    enemy_tanks_alive++;
                }
            }
        }

        score -= enemy_tanks_alive * WIN;
        score += ally_tanks_alive * WIN * 2; // More important to stay alive than to kill enemies
    }
    
    fetch_walls_and_mines(board_copy);
    score += score_position(board_copy, self_copy);

    // Score based on steps since the last board update
    if (stepsSinceBoardUpdate > 3) {
        score -= stepsSinceBoardUpdate * 10; // Penalize for not updating the board frequently
    }

    if (score == DEATH) {
        return DEATH; // Tank is dead
    }

    if (lookahead > 0) {
        double next_score = decide_move(board_copy, self_copy, lookahead - 1, stepsSinceBoardUpdate + 1, false).second;
        if (next_score == DEATH) {
            return DEATH;
        }

        score = score * 0.4 + next_score * 0.6; // Weighted average
    }

    return score;
}


/**
 * Use DFS to find the shortest path from start to end (cannot cross walls).
 */
int find_shortest_path(Vector2D start, Vector2D end, game_board* board) {
    std::vector<std::vector<bool>> visited(board->n, std::vector<bool>(board->m, false));
    std::queue<std::pair<Vector2D, int>> q; // Pair of position and distance
    q.push({start, 0});
    visited[start.x][start.y] = true;

    while (!q.empty()) {
        auto [current, dist] = q.front();
        q.pop();

        if (current.x == end.x && current.y == end.y) {
            return dist;
        }

        for (int dx = -1; dx <= 1; ++dx) {
            for (int dy = -1; dy <= 1; ++dy) {
                if (dx == 0 && dy == 0) {
                    continue; // Skip the current position
                }

                int new_x = (current.x + dx + board->n) % board->n;
                int new_y = (current.y + dy + board->m) % board->m;

                if (!visited[new_x][new_y]) {
                    visited[new_x][new_y] = true;

                    if (board->get_cell(new_x, new_y).has_Object()) {
                        game_object* obj = board->get_cell(new_x, new_y).get_Object();
                        if (obj->get_symbol() == '#' || obj->get_symbol()== '@') {
                            continue; // Skip walls and mines
                        }
                    }

                    Vector2D new_pos = {new_x, new_y};
                    q.push({new_pos, dist + 1});
                }
            }
        }
    }

    return std::numeric_limits<int>::max(); // No path found
}

running_algorithm::running_algorithm() : shell_avoidance_algorithm() {}
double running_algorithm::score_position(game_board* board_copy, shared_ptr<tank> self_copy) {
    double score = shell_avoidance_algorithm::score_position(board_copy, self_copy);

    if (score == DEATH) {
        return DEATH;
    }

    for (auto& t : board_copy->tanks) {
        if (t->player_number!= self_copy->player_number) {
            Vector2D tank_pos = {self_copy->get_x(), self_copy->get_y()};
            Vector2D enemy_tank_pos = {t->get_x(), t->get_y()};

            int shortest_path = find_shortest_path(tank_pos, enemy_tank_pos, board_copy);

            if (shortest_path < 7) {
                score -= pow(10.0 / (double) (shortest_path + 1), 2); // Closer to the enemy tank, lower the score
            }
        }
    }

    return score;
}


chasing_algorithm::chasing_algorithm() : shell_avoidance_algorithm() {}

double chasing_algorithm::score_position(game_board* board_copy, shared_ptr<tank> self_copy) {
    double score = shell_avoidance_algorithm::score_position(board_copy, self_copy);

    if (score == DEATH) {
        return DEATH;
    }

    for (auto& t : board_copy->tanks) {
        if (t->player_number != self_copy->player_number) {
            Vector2D tank_pos = {self_copy->get_x(), self_copy->get_y()};
            Vector2D enemy_tank_pos = {t->get_x(), t->get_y()};

            int shortest_path = find_shortest_path(tank_pos, enemy_tank_pos, board_copy);
            score += pow(15.0 / (double) (shortest_path + 1), 1.5); // Closer to the enemy tank, higher the score
        }
    }

    return score;
}

double chasing_algorithm::score_shoot(game_board* board, shared_ptr<tank> self, int lookahead, int stepsSinceBoardUpdate) {
    if (self->shot_timer > 0) {
        return -1000000; // Cannot shoot
    }

    double score = shell_avoidance_algorithm::score_shoot(board, self, lookahead, stepsSinceBoardUpdate);

    if (score == DEATH) {
        return DEATH;
    }

    for (auto& t : board->tanks) {
        if (t->player_number != self->player_number) {
            // Check if the enemy tank is in the line of fire
            Vector2D enemy_tank_pos = {t->get_x(), t->get_y()};
            Vector2D shell_pos = {self->get_x() + self->directionx, self->get_y() + self->directiony};
            Vector2D shell_dir = {self->directionx, self->directiony};
            std::pair<int, int> dists = chebyshevDistanceToLine(shell_pos, shell_dir, enemy_tank_pos, board);

            if (dists.first == 0) {
                // The enemy tank is in the line of fire
                int distance_to_shell = dists.second;
                score += pow(20.0 / (double) (distance_to_shell + 1), 2);

                if (t->directionx == self->directionx && t->directiony == self->directiony) {
                    // The enemy tank is moving in the same direction as the shell, will be more difficult to evade
                    score += 30; // Increase score for shooting in the same direction
                }
            }
        }
    }

    return score;
}