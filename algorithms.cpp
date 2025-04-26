#include "algorithms.h"

algorithm::algorithm() {}

void algorithm::move_other_tanks(game_board* board, tank* self) {
    for (tank* t : board->tanks) {
        if (t->symbol != self->symbol) {
            t->turn(board, "fw");
        }
    }
}

void algorithm::do_move(game_board* board, tank* self, const std::string& move) {
    board->do_step();
    self->turn(board, move);
    // move_other_tanks(board, self);
}

double algorithm::score_single_move(game_board* board, tank* self, const std::string& move, int lookahead) {
    game_board* board_copy = board->deep_copy();
    tank* self_copy = get_self_in_board_copy(board_copy, self);

    do_move(board_copy, self_copy, move);
    double score = base_score(board_copy, self_copy, lookahead);

    board_copy->destroy_all_objects();
    delete board_copy;

    return score;
}

double algorithm::score_forward_move(game_board* board, tank* self, int lookahead) {
    return score_single_move(board, self, "fw", lookahead);
}

double algorithm::score_backward_move(game_board* board, tank* self, int lookahead) {
    return score_single_move(board, self, "bw", lookahead);
}

double algorithm::score_rotate_left_quarter(game_board* board, tank* self, int lookahead) {
    return score_single_move(board, self, "r4l", lookahead);
}

double algorithm::score_rotate_right_quarter(game_board* board, tank* self, int lookahead) {
    return score_single_move(board, self, "r4r", lookahead);
}

double algorithm::score_rotate_left_eighth(game_board* board, tank* self, int lookahead) {
    return score_single_move(board, self, "r8l", lookahead);
}

double algorithm::score_rotate_right_eighth(game_board* board, tank* self, int lookahead) {
    return score_single_move(board, self, "r8r", lookahead);
}

double algorithm::score_shoot(game_board* board, tank* self, int lookahead) {
    return score_single_move(board, self, "shoot", lookahead);
}

double algorithm::score_skip(game_board* board, tank* self, int lookahead) {
    return score_single_move(board, self, "skip", lookahead);
}

tank* algorithm::get_self_in_board_copy(game_board* board_copy, tank* self) {
    for (tank* t : board_copy->tanks) {
        if (t->symbol == self->symbol) {
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

std::pair<std::string, double> algorithm::decide_move(game_board* board, tank* self, int lookahead) {
    double forward = score_forward_move(board, self, lookahead);
    double backward = score_backward_move(board, self, lookahead);
    double rotate_left_quarter = score_rotate_left_quarter(board, self, lookahead);
    double rotate_right_quarter = score_rotate_right_quarter(board, self, lookahead);
    double rotate_left_eighth = score_rotate_left_eighth(board, self, lookahead);
    double rotate_right_eighth = score_rotate_right_eighth(board, self, lookahead);
    double shoot = score_shoot(board, self, lookahead);
    double skip = score_skip(board, self, lookahead);

    double max_score = std::max({forward, backward, rotate_left_quarter, rotate_right_quarter, rotate_left_eighth, rotate_right_eighth, shoot, skip});

    if (max_score == forward) return {"fw", max_score};
    else if (max_score == backward) return {"bw", max_score};
    else if (max_score == rotate_left_quarter) return {"r4l", max_score};
    else if (max_score == rotate_right_quarter) return {"r4r", max_score};
    else if (max_score == rotate_left_eighth) return {"r8l", max_score};
    else if (max_score == rotate_right_eighth) return {"r8r", max_score};
    else if (max_score == shoot) return {"shoot", max_score};
    else return {"skip", max_score};
}

shell_avoidance_algorithm::shell_avoidance_algorithm() : algorithm(), shell_danger_radius(2), mine_danger_radius(2) {}

double shell_avoidance_algorithm::score_position(game_board* board_copy, tank* self_copy) {
    double score = 0;

    for (shell* s : board_copy->shells) {
        Vector2D shell_pos = {(double) s->x, (double) s->y};
        Vector2D shell_dir = {(double) s->directionx, (double) s->directiony};
        Vector2D tank_pos = {(double) self_copy->x, (double) self_copy->y};

        if (shell_pos.x == tank_pos.x && shell_pos.y == tank_pos.y) {
            return DEATH;
        }

        double dist_to_traj = chebyshevDistanceToLine(shell_pos, shell_dir, tank_pos, board_copy->n, board_copy->m);

        // -1 means the point is "behind" the line
        if (dist_to_traj != -1 && dist_to_traj <= shell_danger_radius) {
            double distance_to_shell = tank_pos.chebyshevDistance(shell_pos);
            score -= (shell_danger_radius + 1 - dist_to_traj) / distance_to_shell;
        }
    }

    for (mine* m : mines) {
        Vector2D mine_pos = {(double) m->x,(double) m->y};
        Vector2D tank_pos = {(double) self_copy->x, (double) self_copy->y};

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

double shell_avoidance_algorithm::base_score(game_board* board_copy, tank* self_copy, int lookahead) {
    // First, check if the tank still exists in the board copy
    bool tank_exists = false;
    for (tank* t : board_copy->tanks) {
        if (t->symbol == self_copy->symbol) {
            tank_exists = true;
            break;
        }
    }
    if (!tank_exists) {
        return DEATH; // Tank does not exist in the board copy
    } else if (board_copy->tanks.size() == 1) {
        return WIN; // Tank is the only one left
    }
    
    fetch_walls_and_mines(board_copy);
    double score = score_position(board_copy, self_copy);

    if (score == DEATH) {
        return DEATH; // Tank is dead
    }

    if (lookahead > 0) {
        double next_score = decide_move(board_copy, self_copy, lookahead - 1).second;
        if (next_score == DEATH) {
            return DEATH;
        }

        // Next score is weighted more heavily because it's where we'll end up
        score = score * 0.15 + next_score * 0.85;
    }

    return score;
}


/**
 * Use DFS to find the shortest path from start to end (cannot cross walls).
 */
double find_shortest_path(Vector2D start, Vector2D end, game_board* board) {
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

                int new_x = ((int)round(current.x) + dx + board->n) % board->n;
                int new_y = ((int)round(current.y) + dy + board->m) % board->m;

                if (new_x >= 0 && new_x < board->n && new_y >= 0 && new_y < board->m && !visited[new_x][new_y]) {
                    if (board->arr[new_x][new_y].has_Object()) {
                        game_object* obj = board->arr[new_x][new_y].get_Object();
                        if (obj->symbol == 'w' || obj->symbol == 'm') {
                            continue; // Skip walls and mines
                        }
                    }
                    visited[new_x][new_y] = true;
                    Vector2D new_pos = {(double) new_x, (double) new_y};
                    q.push({new_pos, dist + 1});
                }
            }
        }
    }

    return std::numeric_limits<double>::max(); // No path found
}


chasing_algorithm::chasing_algorithm() : shell_avoidance_algorithm() {
    shell_danger_radius = 2;
    mine_danger_radius = 2;
}

double chasing_algorithm::score_position(game_board* board_copy, tank* self_copy) {
    double score = shell_avoidance_algorithm::score_position(board_copy, self_copy);

    if (score == DEATH) {
        return DEATH;
    }

    for (tank* t : board_copy->tanks) {
        if (t->symbol != self_copy->symbol) {
            Vector2D tank_pos = {(double) self_copy->x, (double) self_copy->y};
            Vector2D enemy_tank_pos = {(double) t->x, (double) t->y};

            double shortest_path = find_shortest_path(tank_pos, enemy_tank_pos, board_copy);

            if (shortest_path < 7) {
                score += pow(7.0 / max(1.0, shortest_path), 1.5); // Closer to the enemy tank, higher the score
            }

            break;
        }
    }

    return score;
}
