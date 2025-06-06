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
    // std::cout << "[DEBUG] run() started." << std::endl;
    std::string input_basename = std::filesystem::path(input_filename).stem().string();
    std::string output_filename = "output_" + input_basename + ".txt";
    std::ofstream game_output(output_filename);

    if (!game_output.is_open()) {
        std::cerr << "Error opening game log file!" << std::endl;
        return;
    }
    // std::cout << "[DEBUG] Output file created: " << output_filename << std::endl;

    int time_out_steps = 10;
    bool game_over = false;

    while (time_out_steps >= 0 && !game_over) {
        // std::cout << "[DEBUG] Starting new game step. time_out_steps=" << time_out_steps << std::endl;
        satelliteCopyReady = false;

        bool out_of_shells = true;
        for (const auto& t_ptr : board->tanks) {
        tank* t = t_ptr.get();  // get raw pointer from shared_ptr
        if (t->shells > 0) {
            out_of_shells = false;
            break;
        }
}

        if (out_of_shells) {
            time_out_steps--;
            std::cout << "[DEBUG] All tanks are out of shells! Reducing time_out_steps to " << time_out_steps << std::endl;
        }

        for (const auto& t_ptr : board->tanks) {
        tank* t = t_ptr.get();
        if (t->shot_timer > 0) {
            t->shot_timer--;
            std::cout << "[DEBUG] Tank " << t->symbol << "'s shot timer reduced to " << t->shot_timer << std::endl;
        }
}

        std::vector<std::string> moves;
        for (const auto& t_ptr : board->tanks) {
        tank* t = t_ptr.get();
        std::cout << "[DEBUG] Getting move for tank " << t->symbol << std::endl;
        if (!t->alive) {
            moves.push_back("(killed)");
        } else {
            ActionRequest action = t->algo->getAction();
            std::cout << "[DEBUG] ActionRequest received: " << static_cast<int>(action) << std::endl;
            if (action == ActionRequest::GetBattleInfo) {
                int player_index = (t->symbol == '1') ? 0 : 1;
                if (!satelliteCopyReady) {
                    std::cout << "[DEBUG] Updating satellite view copy..." << std::endl;
                    static_cast<SatelliteViewImpl*>(satview.get())->updateCopy(*board);
                    satelliteCopyReady = true;
                }
                std::cout << "[DEBUG] Calling updateTankWithBattleInfo for player " << player_index << std::endl;
                players[player_index]->updateTankWithBattleInfo(*t->algo, *satview);
            }
            moves.push_back(actionToString(action));
        }
    }


        for (size_t i = 0; i < board->tanks.size(); ++i) {
            tank* t = board->tanks[i].get(); 
            const std::string& move = moves[i];
            if (move == "(killed)") {
                std::cout << "[DEBUG] Tank " << t->symbol << " is killed." << std::endl;
                game_output << "Tank " << t->symbol << ": (killed)" << std::endl;
                continue;
            }
            std::cout << "[DEBUG] Tank " << t->symbol << " executes move: " << move << std::endl;
            game_output << "Tank " << t->symbol << ": " << move << std::endl;
            t->turn(board.get(), move);
        }

        std::cout << "[DEBUG] Checking collisions..." << std::endl;
        game_over = board->handle_cell_collisions();
        if (game_over) break;

        std::cout << "[DEBUG] Executing board step..." << std::endl;
        game_over = board->do_step();

        std::cout << "[DEBUG] Printing board state..." << std::endl;
        board->print_board();
    }

    std::cout << "[DEBUG] Game over!" << std::endl;
    if (board->tanks.size() == 1) {
        std::cout << "Player " << board->tanks[0]->symbol << " wins!" << std::endl;
        game_output << "Player " << board->tanks[0]->symbol << " wins!" << std::endl;
    } else {
        std::cout << "It's a tie!" << std::endl;
        game_output << "It's a tie!" << std::endl;
    }

    game_output.close();
    std::cout << "[DEBUG] run() completed." << std::endl;
}
