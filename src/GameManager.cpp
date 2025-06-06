#include "GameManager.h"
#include "TankAlgorithmFactory.h"
#include "MyPlayerFactory.h"
#include "PlayerFactory.h"
#include "Board.h"
#include "SatelliteView.h"
#include "ActionRequest.h"
#include "TankAlgorithm.h"
#include "Player.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <set>
#include <map>
#include <filesystem>
#include <unordered_set>
#include "utils.h"


GameManager::GameManager(std::unique_ptr<PlayerFactory> playerFactory,
                         std::unique_ptr<TankAlgorithmFactory> tankFactory)
    : playerFactory(std::move(playerFactory)),
      tankAlgorithmFactory(std::move(tankFactory)) {
    satview = std::make_unique<SatelliteViewImpl>();
}

void GameManager::readBoard(const std::string& filename) {
    input_filename = filename;
    std::ifstream file(input_filename);

    if (!file.is_open()) {
        std::cerr << "[ERROR] Failed to open file: " << input_filename << std::endl;
        return;
    }

    std::string line;
    getline(file, line); // Skip map description

    int maxSteps = 0, numShells = 0, rows = 0, cols = 0;
    getline(file, line); sscanf(line.c_str(), "MaxSteps = %d", &maxSteps);
    getline(file, line); sscanf(line.c_str(), "NumShells = %d", &numShells);
    getline(file, line); sscanf(line.c_str(), "Rows = %d", &rows);
    getline(file, line); sscanf(line.c_str(), "Cols = %d", &cols);

    std::vector<std::vector<cell>> arr;
    arr.reserve(rows);
    for (int i = 0; i < rows; ++i) {
        std::vector<cell> row;
        row.reserve(cols);
        for (int j = 0; j < cols; ++j) {
            row.emplace_back(i, j);
        }
        arr.push_back(std::move(row));
    }

    board = std::make_unique<game_board>(rows, cols, std::move(arr));
    std::map<char, int> tank_counts;
    std::set<char> valid_chars = {'#', '1', '2', ' ', '@'};

    for (int i = 0; i < rows; ++i) {
        getline(file, line);
        for (int j = 0; j < cols; ++j) {
            char ch = (j < static_cast<int>(line.size())) ? line[j] : ' ';
            if (valid_chars.find(ch) == valid_chars.end()) ch = ' ';

            cell& current = board->arr[i][j];
            current.x = i;
            current.y = j;

            if (ch == '#') {
                current.add_Object(std::make_shared<wall>('w', &current));
            } else if (ch == '@') {
                current.add_Object(std::make_shared<mine>('@', &current));
            } else if (ch == '1' || ch == '2') {
                auto tank_ptr = std::make_shared<tank>(ch, 0, 1, &current, nullptr);
                board->tanks.push_back(tank_ptr);
                current.add_Object(tank_ptr);
                tank_counts[ch]++;
            }
        }
    }

    file.close();

    for (const auto& t_ptr : board->tanks) {
        tank* t = t_ptr.get();
        int player_index = (t->symbol == '1') ? 0 : 1;
        auto algo = tankAlgorithmFactory->create(player_index, 0);
        t->algo = algo.get();
        tankAlgorithms.push_back(std::move(algo));
    }

    players.resize(2);
    players[0] = playerFactory->create(0, rows, cols, maxSteps, numShells);
    players[1] = playerFactory->create(1, rows, cols, maxSteps, numShells);
}


std::string GameManager::actionToString(ActionRequest action) {
    switch (action) {
        case ActionRequest::MoveForward: return "fw";
        case ActionRequest::MoveBackward: return "bw";
        case ActionRequest::RotateLeft90: return "r4l";
        case ActionRequest::RotateRight90: return "r4r";
        case ActionRequest::RotateLeft45: return "r8l";
        case ActionRequest::RotateRight45: return "r8r";
        case ActionRequest::Shoot: return "shoot";
        case ActionRequest::DoNothing: return "skip";
        default: return "skip";
    }
}

std::string GameManager::askAlgorithm(tank* t) {
    if (t->alive && t->algo) {
        ActionRequest action = t->algo->getAction();
        if (action == ActionRequest::GetBattleInfo) {
            int idx = (t->symbol == '1') ? 0 : 1;
            players[idx]->updateTankWithBattleInfo(*t->algo, *satview);
            action = t->algo->getAction();
        }
        return actionToString(action);
    }
    return "skip";
}

void GameManager::run() {
    const int MAX_SHELL_TIMEOUT = 40;
    const int MAX_GAME_STEPS = 50;

    std::string input_basename = std::filesystem::path(input_filename).stem().string();
    std::string output_filename = "output_" + input_basename + ".txt";
    std::ofstream game_output(output_filename);

    if (!game_output.is_open()) {
        std::cerr << "Error opening game log file!" << std::endl;
        return;
    }

    // std::cout << "[DEBUG] Output file: " << output_filename << std::endl;

    int time_out_steps = MAX_SHELL_TIMEOUT;
    bool game_over = false;

    // Order tanks by birth: top-left to bottom-right
    std::vector<tank*> tanks_by_birth;
    for (const auto& t_ptr : board->tanks) {
        tanks_by_birth.push_back(t_ptr.get());
    }
    std::sort(tanks_by_birth.begin(), tanks_by_birth.end(), [](tank* a, tank* b) {
        if (a->get_x() != b->get_x()) return a->get_x() < b->get_x();
        return a->get_y() < b->get_y();
    });

    std::unordered_set<tank*> killed_tanks;
    int round_counter = 0;

    while (time_out_steps >= 0 && !game_over && round_counter < MAX_GAME_STEPS) {
        // std::cout << "\n[DEBUG] ===== Round " << round_counter + 1 << " =====" << std::endl;
        satelliteCopyReady = false;

        bool out_of_shells = true;
        for (const auto& t_ptr : board->tanks) {
            tank* t = t_ptr.get();
            if (t->shells > 0 && t->alive) {
                out_of_shells = false;
                break;
            }
        }

        if (out_of_shells) {
            time_out_steps--;
            std::cout << "[DEBUG] All tanks are out of shells. time_out_steps = " << time_out_steps << std::endl;
        }

        for (tank* t : tanks_by_birth) {
            if (t->shot_timer > 0) {
                t->shot_timer--;
            }
        }

        board->print_board();

        std::vector<std::string> moves;
        std::vector<bool> turn_success;

        for (tank* t : tanks_by_birth) {
            if (!t->alive) {
                moves.push_back("killed");
                turn_success.push_back(false);
                continue;
            }

            ActionRequest action = t->algo->getAction();
            std::string move_str = actionToString(action);
            // std::cout << "[DEBUG] Tank " << t->get_symbol() << " chose action: " << move_str << std::endl;

            if (action == ActionRequest::GetBattleInfo) {
                int player_index = (t->get_symbol() == '1') ? 0 : 1;
                if (!satelliteCopyReady) {
                    std::cout << "[DEBUG] Updating SatelliteView copy..." << std::endl;
                    static_cast<SatelliteViewImpl*>(satview.get())->updateCopy(*board);
                    satelliteCopyReady = true;
                }
                players[player_index]->updateTankWithBattleInfo(*t->algo, *satview);
            }

            moves.push_back(move_str);
            turn_success.push_back(true);
        }

        for (size_t i = 0; i < tanks_by_birth.size(); ++i) {
            tank* t = tanks_by_birth[i];
            if (t->alive && moves[i] != "killed") {
                // std::cout << "[DEBUG] Tank " << t->get_symbol() << " executing move: " << moves[i] << std::endl;
                bool ok = t->turn(board.get(), moves[i]);
                if (!ok) {
                    // std::cout << "[DEBUG] Move failed (ignored): " << moves[i] << std::endl;
                    moves[i] += " (ignored)";
                    turn_success[i] = false;
                }
            }
        }

        // std::cout << "[DEBUG] Handling collisions..." << std::endl;
        game_over = board->handle_cell_collisions();

        for (size_t i = 0; i < tanks_by_birth.size(); ++i) {
            tank* t = tanks_by_birth[i];
            if (!t->alive && !killed_tanks.count(t)) {
                // std::cout << "[DEBUG] Tank " << t->get_symbol() << " was killed." << std::endl;
                if (!turn_success[i] && moves[i].find("(ignored)") != std::string::npos) {
                    moves[i] += " (killed)";
                } else {
                    moves[i] += " (killed)";
                }
                killed_tanks.insert(t);
            }
        }

        // std::cout << "[DEBUG] Round result: " << join(moves, ", ") << std::endl;
        game_output << join(moves, ", ") << std::endl;

        if (!game_over) {
            // std::cout << "[DEBUG] Advancing board state..." << std::endl;
            game_over = board->do_step();
        }

        ++round_counter;
    }

    // std::cout << "[DEBUG] Game loop ended." << std::endl;
    int p1_alive = board->countAliveTanksForPlayer('1');
    int p2_alive = board->countAliveTanksForPlayer('2');

    std::string final_line;
    if (p1_alive == 0 && p2_alive == 0) {
        final_line = "Tie, both players have zero tanks";
    } else if (time_out_steps < 0) {
        final_line = "Tie, both players have zero shells for <" + std::to_string(MAX_SHELL_TIMEOUT) + "> steps";
    } else if (round_counter == MAX_GAME_STEPS) {
        final_line = "Tie, reached max steps = " + std::to_string(MAX_GAME_STEPS) +
                     ", player 1 has " + std::to_string(p1_alive) +
                     " tanks, player 2 has " + std::to_string(p2_alive) + " tanks";
    } else if (p1_alive > p2_alive) {
        final_line = "Player 1 won with " + std::to_string(p1_alive) + " tanks still alive";
    } else if (p2_alive > p1_alive) {
        final_line = "Player 2 won with " + std::to_string(p2_alive) + " tanks still alive";
    } else {
        final_line = "Tie, both players have equal tanks alive";
    }

    // std::cout << "[DEBUG] Final line: " << final_line << std::endl;
    game_output << final_line << std::endl;

    game_output.close();
    // std::cout << "[DEBUG] Output file closed." << std::endl;
}
