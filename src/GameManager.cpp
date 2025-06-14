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
    arr.reserve(cols);
    for (int i = 0; i < cols; ++i) {
        std::vector<cell> col;
        col.reserve(rows);
        for (int j = 0; j < rows; ++j) {
            col.emplace_back(i, j);
        }
        arr.push_back(std::move(col));
    }

    board = std::make_unique<game_board>(cols, rows, std::move(arr));
    std::set<char> valid_chars = {'#', '1', '2', ' ', '@'};

    std::vector<int> tank_counters(2, 0); // 0 for player 1, 1 for player 2

    for (int j = 0; j < rows; ++j) {
        getline(file, line);
        for (int i = 0; i < cols; ++i) {
            char ch = (i < static_cast<int>(line.size())) ? line[i] : ' ';
            if (valid_chars.find(ch) == valid_chars.end()) ch = ' ';

            cell& current = board->get_cell(i, j);
            current.x = i;
            current.y = j;

            if (ch == '#') {
                current.add_Object(std::make_shared<wall>('#', &current));
            } else if (ch == '@') {
                current.add_Object(std::make_shared<mine>('@', &current));
            } else if (ch == '1' || ch == '2') {
                int player_number = (ch == '1') ? 0 : 1;
                int tank_number = tank_counters[player_number]++;
                int directionx = (player_number == 0) ? -1 : 1; // Player 1 faces left, Player 2 faces right
                // Default direction: facing left for player 1, right for player 2
                auto tank_ptr = std::make_shared<tank>(ch, player_number+1, tank_number, directionx, 0, &current, nullptr);
                board->tanks.push_back(tank_ptr);
                current.add_Object(tank_ptr);
            }
        }
    }

    file.close();

    // Assign tank algorithms with correct indices
    for (const auto& t_ptr : board->tanks) {
        tank* t = t_ptr.get();
        int player_index = t->player_number;
        int tank_index = t->tank_number;
        auto algo = tankAlgorithmFactory->create(player_index, tank_index);
        t->algo = algo.get();
        tankAlgorithms.push_back(std::move(algo));
    }

    players.resize(3);
    players[1] = playerFactory->create(1, cols, rows, maxSteps, numShells);
    players[2] = playerFactory->create(2, cols, rows, maxSteps, numShells);
}

std::string actionToString(ActionRequest action) {
    switch (action) {
        case ActionRequest::MoveForward: return "fw";
        case ActionRequest::MoveBackward: return "bw";
        case ActionRequest::RotateLeft90: return "r4l";
        case ActionRequest::RotateRight90: return "r4r";
        case ActionRequest::RotateLeft45: return "r8l";
        case ActionRequest::RotateRight45: return "r8r";
        case ActionRequest::Shoot: return "shoot";
        case ActionRequest::DoNothing: return "skip";
        case ActionRequest::GetBattleInfo: return "update"; // <-- Add this line
        default: return "skip";
    }
}

ActionRequest stringToAction(const std::string& action) {
    if (action == "fw") return ActionRequest::MoveForward;
    if (action == "bw") return ActionRequest::MoveBackward;
    if (action == "r4l") return ActionRequest::RotateLeft90;
    if (action == "r4r") return ActionRequest::RotateRight90;
    if (action == "r8l") return ActionRequest::RotateLeft45;
    if (action == "r8r") return ActionRequest::RotateRight45;
    if (action == "shoot") return ActionRequest::Shoot;
    if (action == "update") return ActionRequest::GetBattleInfo; // <-- Add this line
    if (action == "skip") return ActionRequest::DoNothing;

    return ActionRequest::DoNothing;
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
std::string commandStringToEnumName(const std::string& cmd);
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
    std::unordered_set<tank*> recently_killed; // tanks killed this turn

    // // Helper to print enum as string
    // auto actionEnumToString = [](ActionRequest action) -> std::string {
    //     switch (action) {
    //         case ActionRequest::MoveForward:    return "MoveForward";
    //         case ActionRequest::MoveBackward:   return "MoveBackward";
    //         case ActionRequest::RotateLeft90:   return "RotateLeft90";
    //         case ActionRequest::RotateRight90:  return "RotateRight90";
    //         case ActionRequest::RotateLeft45:   return "RotateLeft45";
    //         case ActionRequest::RotateRight45:  return "RotateRight45";
    //         case ActionRequest::Shoot:          return "Shoot";
    //         case ActionRequest::GetBattleInfo:  return "GetBattleInfo";
    //         case ActionRequest::DoNothing:      return "DoNothing";
    //         default:                            return "Unknown";
    //     }
    // };

    while (time_out_steps >= 0 && !game_over && round_counter < MAX_GAME_STEPS) {
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

        std::vector<ActionRequest> move_enums(tanks_by_birth.size(), ActionRequest::DoNothing);
        std::vector<std::string> moves(tanks_by_birth.size());
        std::vector<bool> turn_success(tanks_by_birth.size(), false);

        // Only ask alive tanks for their move
        for (size_t i = 0; i < tanks_by_birth.size(); ++i) {
            tank* t = tanks_by_birth[i];
            if (t->alive) {
                ActionRequest action = t->algo->getAction();
                move_enums[i] = action;
                moves[i] = actionToString(action); // for internal logic
                turn_success[i] = true;
            } else {
                moves[i] = "killed";
                move_enums[i] = ActionRequest::DoNothing;
            }
        }

        // Execute moves for alive tanks
        for (size_t i = 0; i < tanks_by_birth.size(); ++i) {
            tank* t = tanks_by_birth[i];
            if (t->alive) {
                if (moves[i] == "update") {
                    int player_index = t->player_number;
                    if (!satelliteCopyReady) {
                        static_cast<SatelliteViewImpl*>(satview.get())->updateCopy(*board);
                        satelliteCopyReady = true;
                    }
                    players[player_index]->updateTankWithBattleInfo(*t->algo, *satview);
                    // Do NOT allow another move for this tank this turn
                    continue;
                }
                bool ok = t->turn(board.get(), moves[i]);
                if (!ok) {
                    moves[i] += " (ignored)";
                    turn_success[i] = false;
                }
            }
        }

        // Handle collisions (kills) and fill recently_killed
        recently_killed.clear();
        game_over = board->handle_cell_collisions(&recently_killed);

        if (!game_over) {
            game_over = board->do_step(&recently_killed);
        }

        // Mark moves of tanks killed this turn (AFTER do_step)
        for (size_t i = 0; i < tanks_by_birth.size(); ++i) {
            tank* t = tanks_by_birth[i];
            if (recently_killed.count(t)) {
                moves[i] += " (killed)";
                killed_tanks.insert(t);
            }
        }

        // Output using enum names
        std::vector<std::string> move_names;
        for (size_t i = 0; i < moves.size(); ++i) {
            tank* t = tanks_by_birth[i];
            if (!t->alive) {
                // If it just died this turn, moves[i] already has " (killed)" appended
                if (moves[i].find("(killed)") != std::string::npos) {
                    auto pos = moves[i].find(" (killed)");
                    std::string cmd = moves[i].substr(0, pos);
                    std::string out = commandStringToEnumName(cmd);
                    if (moves[i].find("(ignored)") != std::string::npos)
                        out += " (ignored)";
                    out += " (killed)";
                    move_names.push_back(out);
                } else {
                    move_names.push_back("killed");
                }
            } else {
                std::string out = commandStringToEnumName(moves[i]);
                if (moves[i].find("ignored") != std::string::npos)
                    out += " (ignored)";
                move_names.push_back(out);
            }
        }
        game_output << join(move_names, ", ") << std::endl;
        ++round_counter;
    }

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

    game_output << final_line << std::endl;
    game_output.close();
}

std::string commandStringToEnumName(const std::string& cmd) {
    if (cmd.find("fw") == 0) return "MoveForward";
    if (cmd.find("bw") == 0) return "MoveBackward";
    if (cmd.find("r4l") == 0) return "RotateLeft90";
    if (cmd.find("r4r") == 0) return "RotateRight90";
    if (cmd.find("r8l") == 0) return "RotateLeft45";
    if (cmd.find("r8r") == 0) return "RotateRight45";
    if (cmd.find("shoot") == 0) return "Shoot";
    if (cmd.find("update") == 0) return "GetBattleInfo";
    if (cmd.find("skip") == 0) return "DoNothing";
    return cmd; // fallback for "killed" or unknown
}