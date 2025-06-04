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


class GameManager {
public:
    GameManager(std::unique_ptr<PlayerFactory>, std::unique_ptr<TankAlgorithmFactory>);

    bool readBoard(const std::string& inputFile);
    std::string askAlgorithm(tank* t);
    std::string actionToString(ActionRequest action);

private:
    std::unique_ptr<PlayerFactory> playerFactory;
    std::unique_ptr<TankAlgorithmFactory> tankAlgorithmFactory;
    std::unique_ptr<SatelliteView> satelliteView;
    std::unique_ptr<Player> players[2];
    std::vector<std::unique_ptr<TankAlgorithm>> algorithms;

    game_board board; // ✅ This is the missing line
};
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

    board = game_board(rows, cols);
    std::map<char, int> tank_counts;
    std::set<char> valid_chars = {'#', '1', '2', ' ', '@'};

    for (int i = 0; i < rows; ++i) {
        getline(file, line);
        for (int j = 0; j < cols; ++j) {
            char ch = (j < line.size()) ? line[j] : ' ';
            if (valid_chars.find(ch) == valid_chars.end()) ch = ' ';

            cell& current = board.arr[i][j];
            current.x = i;
            current.y = j;

            if (ch == '#') {
                wall w('w', &current);
            } else if (ch == '@') {
                mine m('@', &current);
            } else if (ch == '1' || ch == '2') {
                tank* t = new tank(ch, 0, 1, &current, nullptr);
                current.add_Object(t);
                board.tanks.push_back(t);
                tank_counts[ch]++;
            }
        }
    }

    file.close();

    if (tank_counts['1'] == 0 || tank_counts['2'] == 0) {
        std::cerr << "Missing tank for one of the players." << std::endl;
        return false;
    }

    for (tank* t : board.tanks) {
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
            players[idx]->updateTankWithBattleInfo(*t->algo, *satelliteView);
            action = t->algo->getAction();
        }
        return actionToString(action);
    }
    return "skip";
}

