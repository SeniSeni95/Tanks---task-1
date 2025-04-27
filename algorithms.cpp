#include "algorithms.h"

algorithm::algorithm() {}

void algorithm::other_tanks_turn(game_board* board, tank* self) {
    for (tank* t : board->tanks) {
        if (t->symbol != self->symbol) {
            t->turn(board, "shoot"); // Assume everyone else tries to kill us
        }
    }
}

void algorithm::do_move(game_board* board, tank* self, const std::string& move) {    
    self->turn(board, move);
    // other_tanks_turn(board, self);
    board->handle_cell_collisions();
    board->do_step();
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
    // Check if the tank can shoot
    if (self->shot_timer > 0) {
        return -100000; // Cannot shoot
    }
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

    // if (lookahead == 2) {
    //     cout << "Forward: " << forward << endl;
    //     cout << "Backward: " << backward << endl;
    //     cout << "Rotate left quarter: " << rotate_left_quarter << endl;
    //     cout << "Rotate right quarter: " << rotate_right_quarter << endl;
    //     cout << "Rotate left eighth: " << rotate_left_eighth << endl;
    //     cout << "Rotate right eighth: " << rotate_right_eighth << endl;
    //     cout << "Shoot: " << shoot << endl;
    //     cout << "Skip: " << skip << endl;
    // }

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

shell_avoidance_algorithm::shell_avoidance_algorithm() : algorithm(), shell_danger_radius(SHELL_DANGER_RADIUS), shell_danger_distance(SHELL_DANGER_DISTANCE), mine_danger_radius(MINE_DANGER_RADIUS) {}

double shell_avoidance_algorithm::score_position(game_board* board_copy, tank* self_copy) {
    double score = 0;

    for (shell* s : board_copy->shells) {
        Vector2D shell_pos = {s->x, s->y};
        Vector2D shell_dir = {s->directionx, s->directiony};
        Vector2D tank_pos = {self_copy->x, self_copy->y};

        if (shell_pos.x == tank_pos.x && shell_pos.y == tank_pos.y) {
            return DEATH;
        }

        // Returns pair of distance from the trajectory line and actual distance to the shell
        std::pair<int, int> dists = chebyshevDistanceToLine(shell_pos, shell_dir, tank_pos, board_copy->n, board_copy->m);

        int distance_to_trajectory = dists.first;
        int distance_to_shell = dists.second;

        if (distance_to_trajectory <= shell_danger_radius && distance_to_shell <= shell_danger_distance) {
            // cout << "Distance to trajectory: " << distance_to_trajectory << ", Distance to shell: " << distance_to_shell << endl;
            score -= pow(20.0 / (double) (distance_to_trajectory + 1), 2); // Avoid being on the trajectory line at all costs
            score -= 20.0 / (double) (distance_to_shell + 1); // Also try getting away from the shell, but not as important
        }
    }

    for (mine* m : mines) {
        Vector2D mine_pos = {m->x,m->y};
        Vector2D tank_pos = {self_copy->x, self_copy->y};

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

    double score = 0;

    if (!tank_exists) {
        return DEATH; // Tank does not exist in the board copy
    } else if (board_copy->tanks.size() == 1) {
        score += WIN; // Tank is the only one left
    }
    
    fetch_walls_and_mines(board_copy);
    score += score_position(board_copy, self_copy);

    if (score == DEATH) {
        return DEATH; // Tank is dead
    }

    if (lookahead > 0) {
        double next_score = decide_move(board_copy, self_copy, lookahead - 1).second;
        if (next_score == DEATH) {
            return DEATH;
        }

        score = score * 0.2 + next_score * 0.8;
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
                    Vector2D new_pos = {new_x, new_y};
                    q.push({new_pos, dist + 1});
                }
            }
        }
    }

    return std::numeric_limits<int>::max(); // No path found
}

running_algorithm::running_algorithm() : shell_avoidance_algorithm() {}
double running_algorithm::score_position(game_board* board_copy, tank* self_copy) {
    double score = shell_avoidance_algorithm::score_position(board_copy, self_copy);

    if (score == DEATH) {
        return DEATH;
    }

    for (tank* t : board_copy->tanks) {
        if (t->symbol != self_copy->symbol) {
            Vector2D tank_pos = {self_copy->x, self_copy->y};
            Vector2D enemy_tank_pos = {t->x, t->y};

            int shortest_path = find_shortest_path(tank_pos, enemy_tank_pos, board_copy);

            if (shortest_path < 7) {
                score -= pow(10.0 / (double) (shortest_path + 1), 2); // Closer to the enemy tank, lower the score
            }

            break;
        }
    }

    return score;
}


chasing_algorithm::chasing_algorithm() : shell_avoidance_algorithm() {}

double chasing_algorithm::score_position(game_board* board_copy, tank* self_copy) {
    double score = shell_avoidance_algorithm::score_position(board_copy, self_copy);

    if (score == DEATH) {
        return DEATH;
    }

    for (tank* t : board_copy->tanks) {
        if (t->symbol != self_copy->symbol) {
            Vector2D tank_pos = {self_copy->x, self_copy->y};
            Vector2D enemy_tank_pos = {t->x, t->y};

            int shortest_path = find_shortest_path(tank_pos, enemy_tank_pos, board_copy);

            if (shortest_path < 7) {
                score += 5.0 / (double) (shortest_path + 1); // Closer to the enemy tank, higher the score
            }

            break;
        }
    }

    return score;
}

double chasing_algorithm::score_shoot(game_board* board, tank* self, int lookahead) {
    if (self->shot_timer > 0) {
        return -1000000; // Cannot shoot
    }

    double score = shell_avoidance_algorithm::score_shoot(board, self, lookahead);

    if (score == DEATH) {
        return DEATH;
    }

    for (tank* t : board->tanks) {
        if (t->symbol != self->symbol) {
            // Check if the enemy tank is in the line of fire
            Vector2D tank_pos = {self->x, self->y};
            Vector2D enemy_tank_pos = {t->x, t->y};
            Vector2D shell_pos = {self->x + self->directionx, self->y + self->directiony};
            Vector2D shell_dir = {self->directionx, self->directiony};
            std::pair<int, int> dists = chebyshevDistanceToLine(shell_pos, shell_dir, enemy_tank_pos, board->n, board->m);
            if (dists.first == 0) {
                // The enemy tank is in the line of fire
                int distance_to_shell = dists.second;
                score += 8.0 / (double) (distance_to_shell + 1);

                if (t->directionx == self->directionx && t->directiony == self->directiony) {
                    // The enemy tank is moving in the same direction as the shell, will be more difficult to evade
                    score += 30; // Increase score for shooting in the same direction
                }
            }
        }
    }

    return score;
}