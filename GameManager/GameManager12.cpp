#include "GameManager12.h"  // <-- Fixed include
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


GameManager12::GameManager12(std::unique_ptr<PlayerFactory> playerFactory,  // <-- Fixed class name and parameter type
                           MyTankAlgorithmFactory tankFactory,
                           bool verbose)
    : playerFactory(std::move(playerFactory)),
      myTankAlgorithmFactory(std::move(tankFactory)), // use lowercase variable name to match header
      verboseOutput(verbose) {
    satview = std::make_unique<SatelliteViewImpl>();
    if (verboseOutput) {
        std::string outName = "output_verbose.txt";
        verboseFile.open(outName);
        if (!verboseFile.is_open()) {
            std::cerr << "[ERROR] could not open verbose log file" << std::endl;
            verboseOutput = false;
        }
    }
}

GameResult GameManager12::run(  // <-- Fixed class name
    size_t map_width, size_t map_height,
    const SatelliteView& map,
    std::string map_name,
    size_t max_steps, size_t num_shells,
    Player& player1, std::string name1,
    Player& player2, std::string name2,
    TankAlgorithmFactory player1_tank_algo_factory,  // <-- Fixed parameter type to match base class
    TankAlgorithmFactory player2_tank_algo_factory   // <-- Fixed parameter type to match base class
) {
    // --- build board from SatelliteView
   std::vector<std::vector<cell>> arr;
arr.reserve(map_width);
for (size_t i = 0; i < map_width; ++i) {
    std::vector<cell> row;
    row.reserve(map_height);
    for (size_t j = 0; j < map_height; ++j) {
        row.emplace_back((int)i, (int)j);
        char c = map.getObjectAt(i,j);
        if (c == '#') row.back().add_Object(std::make_shared<wall>('#', &row.back()));
        else if (c == '@') row.back().add_Object(std::make_shared<mine>('@', &row.back()));
    }
    arr.push_back(std::move(row));
}

    board = std::make_unique<game_board>((int)map_width,(int)map_height,std::move(arr));

    // (optional) open verbose log file
    std::ofstream game_output;
    if (verboseOutput) {
        std::string output_filename = "output_" + map_name + ".txt";
        game_output.open(output_filename);
        if (!game_output.is_open()) {
            std::cerr << "[ERROR] Failed to open verbose log file: " << output_filename << std::endl;
            verboseOutput = false;
        }
    }

    int time_out_steps = 3;  // <-- FIXED: 3 max turns logic instead of num_shells
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

    while (time_out_steps >= 0 && !game_over && round_counter < (int)max_steps) {
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
            if (verboseOutput) {
                game_output << "[DEBUG] All tanks are out of shells. time_out_steps = "
                            << time_out_steps << std::endl;
            }
        }

        for (tank* t : tanks_by_birth) {
            if (t->shot_timer > 0) t->shot_timer--;
        }

        if (verboseOutput) board->print_board();

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
            } else {
                moves[i] = "killed";
                move_enums[i] = ActionRequest::DoNothing;
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
                        if (t->symbol == '1')
                            player1.updateTankWithBattleInfo(*t->algo, *satview);
                        else
                            player2.updateTankWithBattleInfo(*t->algo, *satview);
                        continue; // skip actual move this turn
                    }
                bool ok = t->turn(board.get(), moves[i]);
                if (!ok) {
                    moves[i] += " (ignored)";
                    turn_success[i] = false;
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
            }
        }

        // Output turn summary if verbose
        if (verboseOutput) {
            std::vector<std::string> move_names;
            for (size_t i = 0; i < moves.size(); ++i) {
                tank* t = tanks_by_birth[i];
                if (!t->alive) {
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
        }

        ++round_counter;
    }

    // Count survivors
    int p1_alive = board->countAliveTanksForPlayer('1');
    int p2_alive = board->countAliveTanksForPlayer('2');

    // Fill result
    GameResult result;
    result.rounds = round_counter;   // <-- your struct uses "rounds", not "steps"

    // Winner: 1 = player1, 2 = player2, 0 = tie
    if (p1_alive > p2_alive) result.winner = 1;
    else if (p2_alive > p1_alive) result.winner = 2;
    else result.winner = 0; // tie

    if (round_counter >= (int)max_steps)
        result.reason = GameResult::MAX_STEPS;
    else if (p1_alive == 0 && p2_alive == 0)
        result.reason = GameResult::ALL_TANKS_DEAD;
    else
        result.reason = GameResult::ZERO_SHELLS;

    // fill remaining tanks
    result.remaining_tanks = { (size_t)p1_alive, (size_t)p2_alive };

    // snapshot of final board (if you want to keep game state)
    if (satview) {
        result.gameState = std::move(satview); // transfer ownership into result
    }

    // Final verbose message
    if (verboseOutput) {
        if (result.winner == 0)  // Changed from -1 to 0 for tie
            game_output << "Tie\n";
        else
            game_output << "Winner: Player " << result.winner << "\n";  // Removed +1
    }

    return result;
}

std::string GameManager12::commandStringToEnumName(const std::string& cmd) {  // <-- Fixed class name
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

std::string GameManager12::actionToString(ActionRequest action) {  // <-- Add missing method implementation
    // Implementation for actionToString method
    switch (action) {
        case ActionRequest::MoveForward: return "fw";
        case ActionRequest::MoveBackward: return "bw";
        case ActionRequest::RotateLeft90: return "r4l";
        case ActionRequest::RotateRight90: return "r4r";
        case ActionRequest::RotateLeft45: return "r8l";
        case ActionRequest::RotateRight45: return "r8r";
        case ActionRequest::Shoot: return "shoot";
        case ActionRequest::GetBattleInfo: return "update";
        case ActionRequest::DoNothing: return "skip";
        default: return "skip";
    }
}

ActionRequest GameManager12::stringToAction(const std::string& actionStr) {  // <-- Add missing method implementation
    // Implementation for stringToAction method
    if (actionStr == "fw") return ActionRequest::MoveForward;
    if (actionStr == "bw") return ActionRequest::MoveBackward;
    if (actionStr == "r4l") return ActionRequest::RotateLeft90;
    if (actionStr == "r4r") return ActionRequest::RotateRight90;
    if (actionStr == "r8l") return ActionRequest::RotateLeft45;
    if (actionStr == "r8r") return ActionRequest::RotateRight45;
    if (actionStr == "shoot") return ActionRequest::Shoot;
    if (actionStr == "update") return ActionRequest::GetBattleInfo;
    if (actionStr == "skip") return ActionRequest::DoNothing;
    return ActionRequest::DoNothing;
}
