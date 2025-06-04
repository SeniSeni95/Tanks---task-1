#include "GameManager.h"
#include "TankAlgorithmFactory.h"
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
// using namespace std;



bool GameManager::readBoard(const std::string& inputFile) {
    std::ifstream file(inputFile);
    if (!file.is_open()) {
        std::cerr << "Failed to open input file." << std::endl;
        return false;
    }

    std::string line;
    getline(file, line); // skip map name/description

    int maxSteps = 0, numShells = 0, rows = 0, cols = 0;

    getline(file, line); sscanf(line.c_str(), "MaxSteps = %d", &maxSteps);
    getline(file, line); sscanf(line.c_str(), "NumShells = %d", &numShells);
    getline(file, line); sscanf(line.c_str(), "Rows = %d", &rows);
    getline(file, line); sscanf(line.c_str(), "Cols = %d", &cols);

    // Create and populate the 2D vector of cells
    std::vector<std::vector<cell>> arr(rows, std::vector<cell>(cols));
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            arr[i][j].x = i;
            arr[i][j].y = j;
        }
    }


   board = std::make_unique<game_board>(rows, cols, arr);
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
                wall w('w', &current);
            } else if (ch == '@') {
                mine m('@', &current);
            } else if (ch == '1' || ch == '2') {
                tank* t = new tank(ch, 0, 1, &current, nullptr);
                current.add_Object(t);
                board->tanks.push_back(t); 
                tank_counts[ch]++;
            }
        }
    }

    file.close();

    if (tank_counts['1'] == 0 || tank_counts['2'] == 0) {
        std::cerr << "Missing tank for one of the players." << std::endl;
        return false;
    }

    for (tank* t : board->tanks) {
    int player_index = (t->symbol == '1') ? 0 : 1;
    auto algo = tankAlgorithmFactory->create(player_index, 0);  // fix tank index later
    t->algo = algo.get();  // assign raw pointer
    algorithms.push_back(std::move(algo));  // store unique_ptr to retain ownership
}

    players[0] = playerFactory->create(0, rows, cols, maxSteps, numShells);
    players[1] = playerFactory->create(1, rows, cols, maxSteps, numShells);

    return true;
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


void GameManager::run(const std::string& input_filename) {
    std::string output_filename = "output_" + input_filename.substr(0, input_filename.find('.')) + ".txt";
    std::ofstream game_output(output_filename);
    if (!game_output.is_open()) {
        std::cerr << "Error opening game log file!" << std::endl;
        return;
    }

    int time_out_steps = 10;
    bool game_over = false;

    while (time_out_steps >= 0 && !game_over) {
        satelliteCopyReady = false;
        bool out_of_shells = true;
        for (tank* t : board->tanks) {
            if (t->shells > 0) {
                out_of_shells = false;
                break;
            }
        }

        if (out_of_shells) {
            time_out_steps--;
            std::cout << "All tanks are out of shells! Reducing time_out_steps to " << time_out_steps << std::endl;
        }

        for (tank* t : board->tanks) {
            if (t->shot_timer > 0) {
                t->shot_timer--;
                std::cout << "Tank " << t->symbol << "'s shot timer reduced to " << t->shot_timer << std::endl;
            }
        }

         std::vector<std::string> moves;
            for (tank* t : board->tanks) {
            if (!t->alive) {
                moves.push_back("(killed)");
            } else {
            ActionRequest action = t->algo->getAction();
                if (action == ActionRequest::GetBattleInfo) {
                     int player_index = (t->symbol == '1') ? 0 : 1;
                        if (!satelliteCopyReady) {
                            satelliteViewImpl.updateCopy(*board);
                            satelliteCopyReady = true;
                        }
               players[player_index]->updateTankWithBattleInfo(*t->algo, satelliteViewImpl);
}
                moves.push_back(actionToString(action));
            }

        for (size_t i = 0; i < board->tanks.size(); ++i) {
            tank* t = board->tanks[i];
            const std::string& move = moves[i];
            if (move == "(killed)") {
                std::cout << "Tank " << t->symbol << " is killed." << std::endl;
                game_output << "Tank " << t->symbol << ": (killed)" << std::endl;
                continue;
            }
            game_output << "Tank " << t->symbol << ": " << move << std::endl;
            t->turn(board.get(), move);
        }

        game_over = board->handle_cell_collisions();
        if (game_over) break;

        game_over = board->do_step();
        board->print_board();
    }

    std::cout << "Game over!" << std::endl;
    if (board->tanks.size() == 1) {
        std::cout << "Player " << board->tanks[0]->symbol << " wins!" << std::endl;
        game_output << "Player " << board->tanks[0]->symbol << " wins!" << std::endl;
    } else {
        std::cout << "It's a tie!" << std::endl;
        game_output << "It's a tie!" << std::endl;
    }

    game_output.close();
}
}
