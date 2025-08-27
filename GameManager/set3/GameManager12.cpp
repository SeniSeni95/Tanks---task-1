#include "GameManager12.h"
#include "../algorithm/MyTankAlgorithmFactory.h"  // Fix include path
#include "../algorithm/MyPlayerFactory.h"
#include "../common/PlayerFactory.h"         // Fix include path
#include "../GameManager/Board.h"            // Fix include path
#include "../common/SatelliteView.h"         // Fix include path
#include "../common/ActionRequest.h"         // Fix include path
#include "../common/TankAlgorithm.h"         // Fix include path
#include "../common/Player.h"                // Fix include path
#include "../common/ActionUtils.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <set>
#include <map>
#include <filesystem>
#include <unordered_set>
#include "utils.h"                 // Fix include path

// Debug control - set to true to enable debugging, false to disable
static const bool DEBUG_ENABLED = false;

GameManager12::GameManager12(PlayerFactory playerFactory,
                         MyTankAlgorithmFactory tankFactory,
                         bool verbose)
    :  playerFactory(std::move(playerFactory)),
      myTankAlgorithmFactory(std::move(tankFactory)), // use lowercase variable name to match header
      verboseOutput(verbose) {
    satview = std::make_unique<SatelliteViewImpl>();
    if (verboseOutput) {
        std::string outName = "output_verbose_gm122.txt";
        verboseFile.open(outName);
        if (!verboseFile.is_open()) {
            std::cerr << "[ERROR] gm122: could not open verbose log file" << std::endl;
            verboseOutput = false;
        }
    }
}

GameResult GameManager12::run(
    size_t map_width, size_t map_height,
    const SatelliteView& map,
    std::string map_name,
    size_t max_steps, size_t num_shells,
    Player& player1, std::string name1,
    Player& player2, std::string name2,
    MyTankAlgorithmFactory player1_tank_algo_factory,
    MyTankAlgorithmFactory player2_tank_algo_factory
) {
    std::vector<int> tank_counters(3, 0);
    if (DEBUG_ENABLED) {
        std::cout << "[DEBUG] gm122: Building board from SatelliteView...\n";
    }

// Prepare cells
std::vector<std::vector<cell>> arr;
arr.reserve(map_width);

// Temporary storage for tanks + algos
std::vector<std::shared_ptr<tank>> tempTanks;
std::vector<std::unique_ptr<TankAlgorithm>> tempAlgos;

// Build grid from SatelliteView
for (size_t i = 0; i < map_width; ++i) {
    std::vector<cell> row;
    row.reserve(map_height);
    for (size_t j = 0; j < map_height; ++j) {
        row.emplace_back((int)i, (int)j);
        char c = map.getObjectAt(i, j);

        if (c == '#') {
            row.back().add_Object(std::make_shared<wall>('#', &row.back()));
        }
        else if (c == '@') {
            row.back().add_Object(std::make_shared<mine>('@', &row.back()));
        }
       else if (c == '1' || c == '2') {
    int player_idx = (c == '1' ? 0 : 1);   // 0 for Player1, 1 for Player2
    int tank_number = ++tank_counters[player_idx];

    int directionx = (player_idx == 0) ? -1 : 1;  // Player1 faces left, Player2 faces right

    // Tanks use player_number = 1/2 (NOT 0/1), just like Task 2
    auto t = std::make_shared<tank>(c, player_idx + 1, tank_number,
                                    directionx, 0, &row.back(), nullptr);
    row.back().add_Object(t);
    tempTanks.push_back(t);

    // Algorithms always created with 0/1 indices (NOT 1/2)
  auto algo = (player_idx == 0
    ? player1_tank_algo_factory(1, tank_number)  // Player 1
    : player2_tank_algo_factory(2, tank_number)  // Player 2
);

    if (algo) {
        t->algo = algo.get();
        tempAlgos.push_back(std::move(algo));
    } else {
        if (DEBUG_ENABLED) {
            std::cerr << "[ERROR] gm122: Tank factory returned nullptr for P"
                      << (player_idx + 1) << " T" << tank_number << "\n";
        }
    }
}

    }
    arr.push_back(std::move(row));
}

// Create the board
board = std::make_unique<game_board>((int)map_width, (int)map_height, std::move(arr));

// Move tanks/algorithms into GameManager's storage
for (auto& t : tempTanks) {
    board->tanks.push_back(t);
}
for (auto& algo : tempAlgos) {
    tankAlgorithms.push_back(std::move(algo));
}

if (DEBUG_ENABLED) {
    std::cout << "[DEBUG] gm122: Board created, tanks=" << board->tanks.size() << "\n";
}

// 🔑 Initialize SatelliteView with this new board
if (satview) {
    static_cast<SatelliteViewImpl*>(satview.get())->updateCopy(*board);
    satelliteCopyReady = true;
    if (DEBUG_ENABLED) {
        std::cout << "[DEBUG] gm122: SatelliteView initialized with board copy\n";
    }
}

    std::ofstream game_output;
    if (verboseOutput) {
        std::string output_filename = "output_gm122_" + map_name + ".txt";
        game_output.open(output_filename);
        if (!game_output.is_open()) {
            std::cerr << "[ERROR] gm122: Failed to open verbose log file: " << output_filename << std::endl;
            verboseOutput = false;
        }
    }

    int time_out_steps = (int)num_shells;
    bool game_over = false;

    // Order tanks by birth
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
    std::unordered_set<tank*> recently_killed;

    if (DEBUG_ENABLED) {
        std::cout << "[DEBUG] gm122: Entering game loop (max_steps=" << max_steps 
                  << ", num_shells=" << num_shells << ")\n";
    }

    while (time_out_steps >= 0 && !game_over && round_counter < 6) {
        if (DEBUG_ENABLED) {
            std::cout << "\n[DEBUG] gm122: --- Round " << (round_counter+1) << " ---\n";
        }
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
            if (DEBUG_ENABLED) {
                std::cout << "[DEBUG] gm122: All tanks are out of shells. time_out_steps=" 
                          << time_out_steps << "\n";
            }
        }

        for (tank* t : tanks_by_birth) {
            if (t->shot_timer > 0) t->shot_timer--;
        }

        std::vector<ActionRequest> move_enums(tanks_by_birth.size(), ActionRequest::DoNothing);
        std::vector<std::string> moves(tanks_by_birth.size());
        std::vector<bool> turn_success(tanks_by_birth.size(), false);

        // Ask each alive tank for move
        for (size_t i = 0; i < tanks_by_birth.size(); ++i) {
            tank* t = tanks_by_birth[i];
            if (t->alive) {
                ActionRequest action = t->algo->getAction();
                move_enums[i] = action;
                moves[i] = actionToString(action);
                turn_success[i] = true;
                if (DEBUG_ENABLED) {
                    std::cout << "[DEBUG] gm122: Tank P" << t->player_number 
                              << " T" << t->tank_number 
                              << " chose: " << moves[i] << "\n";
                }
            } else {
                moves[i] = "killed";
                move_enums[i] = ActionRequest::DoNothing;
                if (DEBUG_ENABLED) {
                    std::cout << "[DEBUG] gm122: Tank P" << t->player_number 
                              << " T" << t->tank_number << " already dead\n";
                }
            }
        }

        // Execute moves
        for (size_t i = 0; i < tanks_by_birth.size(); ++i) {
            tank* t = tanks_by_birth[i];
            if (t->alive) {
                if (moves[i] == "update") {
                    if (!satelliteCopyReady) {
                        static_cast<SatelliteViewImpl*>(satview.get())->updateCopy(*board);
                        satelliteCopyReady = true;
                    }
                  if (t->player_number == 1) {
                    player1.updateTankWithBattleInfo(*t->algo, *satview);
                } else if (t->player_number == 2) {
                    player2.updateTankWithBattleInfo(*t->algo, *satview);
                } else {
                    throw std::runtime_error("Invalid player_number for tank");
                }
                continue;
                }
                bool ok = t->turn(board.get(), moves[i]);
                if (!ok) {
                    moves[i] += " (ignored)";
                    turn_success[i] = false;
                    if (DEBUG_ENABLED) {
                        std::cout << "[DEBUG] gm122: Tank P" << t->player_number 
                                  << " T" << t->tank_number 
                                  << " invalid move: " << moves[i] << "\n";
                    }
                }
            }
        }

        // Handle collisions and steps
        recently_killed.clear();
        game_over = board->handle_cell_collisions(&recently_killed);
        if (!game_over) {
            game_over = board->do_step(&recently_killed);
        }

        // Mark killed tanks
        for (size_t i = 0; i < tanks_by_birth.size(); ++i) {
            tank* t = tanks_by_birth[i];
            if (recently_killed.count(t)) {
                moves[i] += " (killed)";
                killed_tanks.insert(t);
                if (DEBUG_ENABLED) {
                    std::cout << "[DEBUG] gm122: Tank P" << t->player_number 
                              << " T" << t->tank_number << " was killed\n";
                }
            }
        }

        ++round_counter;

        if (DEBUG_ENABLED) {
            int alive1 = board->countAliveTanksForPlayer('1');
            int alive2 = board->countAliveTanksForPlayer('2');
            std::cout << "[DEBUG] gm122: End of round " << round_counter 
                      << " | Alive: P1=" << alive1 << ", P2=" << alive2 << "\n";
        }
    }

    // Count survivors
    int p1_alive = board->countAliveTanksForPlayer('1');
    int p2_alive = board->countAliveTanksForPlayer('2');

    GameResult result;
    result.rounds = round_counter;

    if (p1_alive > p2_alive) result.winner = 1;
    else if (p2_alive > p1_alive) result.winner = 2;
    else result.winner = 0;

    if (round_counter >= (int)max_steps)
        result.reason = GameResult::MAX_STEPS;
    else if (p1_alive == 0 && p2_alive == 0)
        result.reason = GameResult::ALL_TANKS_DEAD;
    else
        result.reason = GameResult::ZERO_SHELLS;

    result.remaining_tanks = { (size_t)p1_alive, (size_t)p2_alive };

    if (satview) {
        result.gameState = std::move(satview);
    }

    if (DEBUG_ENABLED) {
        std::cout << "[DEBUG] gm122: Game finished after " << result.rounds 
                  << " rounds | Winner=" << result.winner 
                  << " | Reason=" << (result.reason == GameResult::ALL_TANKS_DEAD ? "ALL_TANKS_DEAD" :
                                      result.reason == GameResult::MAX_STEPS ? "MAX_STEPS" :
                                      result.reason == GameResult::ZERO_SHELLS ? "ZERO_SHELLS" : "UNKNOWN")
                  << " | P1 alive=" << p1_alive 
                  << " | P2 alive=" << p2_alive << "\n";
    }

    return result;
}

std::string GameManager12::commandStringToEnumName(const std::string& cmd) {
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


