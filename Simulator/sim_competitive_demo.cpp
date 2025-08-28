#include <iostream>
#include <memory>
#include <chrono>
#include <vector>
#include <thread>
#include <mutex>
#include <string>
#include <fstream>
#include <sstream>
#include <set>
#include <filesystem>
#include <algorithm>
#include <map>

#include "../Algorithm/tanks/AggressiveTank.h"
#include "../Algorithm/tanks/CalmTank.h"
#include "MyPlayerFactory.h"
#include "MyTankAlgorithmFactory.h"

#include "GameResult.h"
#include "SatelliteViewImpl.h"
#include "Player.h"
#include "Board.h"
#include "GameObject.h"
#include "../GameManager/GameManager.h"     // single GameManager
#include "../common/AbstractGameManager.h"

static const bool DEBUG_ENABLED = false;
namespace fs = std::filesystem;

//----------------------------------
// LoadedMap + buildMapFromFile
//----------------------------------
struct LoadedMap {
    SatelliteViewImpl view;
    std::unique_ptr<game_board> board;
    std::unique_ptr<Player> p1;
    std::unique_ptr<Player> p2;
    size_t width{};
    size_t height{};
    size_t maxSteps{};
    size_t numShells{};
};

LoadedMap buildMapFromFile(const std::string& filename) {
    LoadedMap lm;
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "[ERROR] Failed to open map file: " << filename << std::endl;
        return lm;
    }

    std::string line;
    getline(file, line); // skip description
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

    auto board = std::make_unique<game_board>(cols, rows, std::move(arr));
    std::set<char> valid_chars = {'#', '1', '2', ' ', '@'};
    std::vector<int> tank_counters(2, 0);

    for (int j = 0; j < rows; ++j) {
        getline(file, line);
        for (int i = 0; i < cols; ++i) {
            char ch = (i < (int)line.size()) ? line[i] : ' ';
            if (valid_chars.find(ch) == valid_chars.end()) ch = ' ';
            cell& current = board->get_cell(i, j);
            if (ch == '#') {
                current.add_Object(std::make_shared<wall>('#', &current));
            } else if (ch == '@') {
                current.add_Object(std::make_shared<mine>('@', &current));
            } else if (ch == '1' || ch == '2') {
                int player_number = (ch == '1') ? 0 : 1;
                int tank_number = ++tank_counters[player_number];
                int dirx = (player_number == 0) ? -1 : 1;
                auto tank_ptr = std::make_shared<tank>(
                    ch, player_number + 1, tank_number,
                    dirx, 0, &current, nullptr
                );
                board->tanks.push_back(tank_ptr);
                current.add_Object(tank_ptr);
            }
        }
    }
    file.close();

    MyPlayerFactory pf;
    auto p1 = pf.create(1, cols, rows, maxSteps, numShells);
    auto p2 = pf.create(2, cols, rows, maxSteps, numShells);

    lm.width = cols;
    lm.height = rows;
    lm.maxSteps = maxSteps;
    lm.numShells = numShells;
    lm.view.updateCopy(*board);
    lm.board = std::move(board);
    lm.p1 = std::move(p1);
    lm.p2 = std::move(p2);
    return lm;
}

//----------------------------------
// Helpers
//----------------------------------
std::string makeOutputFilename(const std::string& folder) {
    auto now = std::chrono::system_clock::now();
    auto t   = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << folder << "/competition_" << t << ".txt";
    return ss.str();
}
std::string reasonToString(GameResult::Reason r) {
    switch (r) {
        case GameResult::ALL_TANKS_DEAD: return "ALL_TANKS_DEAD";
        case GameResult::MAX_STEPS:      return "MAX_STEPS";
        case GameResult::ZERO_SHELLS:    return "ZERO_SHELLS";
        default:                         return "UNKNOWN";
    }
}
std::string winnerToString(int w) {
    if (w == 1) return "Player 1";
    if (w == 2) return "Player 2";
    return "Tie";
}

//----------------------------------
// Worker for threading
//----------------------------------
void runOnMap(const std::string& mapfile,
              MyPlayerFactory& pf,
              MyTankAlgorithmFactory& tankFactoryAggressive,
              MyTankAlgorithmFactory& tankFactoryCalm,
              int& scoreAggressive,
              int& scoreCalm,
              std::mutex& scoreMutex)
{
                LoadedMap lm = buildMapFromFile(mapfile);
                PlayerFactory playerFactory = [&pf](int p, size_t x, size_t y, size_t maxS, size_t numS) {
                return pf.create(p, x, y, maxS, numS);
            };

            GameManager gm(playerFactory, tankFactoryAggressive, false);

    GameResult res = gm.run(
        lm.width, lm.height, lm.view, mapfile,
        lm.maxSteps, lm.numShells,
        *lm.p1, "Aggressive", *lm.p2, "Calm",
        tankFactoryAggressive, tankFactoryCalm
    );

    std::cout << "[Map " << mapfile << "] "
              << "Winner: " << winnerToString(res.winner)
              << " | Reason: " << reasonToString(res.reason)
              << " | Rounds: " << res.rounds << "\n";

    std::lock_guard<std::mutex> lock(scoreMutex);
    if (res.winner == 1) scoreAggressive += 3;
    else if (res.winner == 2) scoreCalm += 3;
    else { scoreAggressive += 1; scoreCalm += 1; }
}

//----------------------------------
// Main
//----------------------------------
int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <maps_folder> [num_threads=N]\n";
        return 1;
    }

    std::string mapsFolder = argv[1];
    int numThreads = 1;
    if (argc >= 3) {
        std::string arg = argv[2];
        if (arg.rfind("num_threads=", 0) == 0)
            numThreads = std::stoi(arg.substr(12));
    }

    std::cout << "=== DEMO SIMULATOR (Competition Run) ===\n";
    std::cout << "Using up to " << numThreads << " threads\n";

    // Scan maps
    std::vector<std::string> maps;
    for (auto& entry : fs::directory_iterator(mapsFolder)) {
    if (entry.is_regular_file() && entry.path().extension() == ".txt") {
        std::string fname = entry.path().filename().string();
        if (fname.rfind("competition_", 0) == 0) continue; // skip output files
        maps.push_back(entry.path().string());
    }
}
    if (maps.empty()) {
        std::cerr << "Error: no maps found in " << mapsFolder << "\n";
        return 1;
    }

    // Scoreboard
    int scoreAggressive = 0;
    int scoreCalm = 0;
    std::mutex scoreMutex;

    // Factories
    MyPlayerFactory pf;
    MyTankAlgorithmFactory tankFactoryAggressive = [](int player, int tank) -> std::unique_ptr<TankAlgorithm> {
        std::unique_ptr<AbstractTankAlgorithm> algo = std::make_unique<AggressiveTank>(player, tank);
        algo->initialize();
        return algo;
    };
    MyTankAlgorithmFactory tankFactoryCalm = [](int player, int tank) -> std::unique_ptr<TankAlgorithm> {
        std::unique_ptr<AbstractTankAlgorithm> algo = std::make_unique<CalmTank>(player, tank);
        algo->initialize();
        return algo;
    };

    // Threaded loop
    std::vector<std::thread> threads;
    size_t active = 0;
    for (size_t i = 0; i < maps.size(); i++) {
        if (numThreads <= 1) {
            runOnMap(maps[i], pf, tankFactoryAggressive, tankFactoryCalm,
                     scoreAggressive, scoreCalm, scoreMutex);
        } else {
            threads.emplace_back(runOnMap, maps[i],
                                 std::ref(pf),
                                 std::ref(tankFactoryAggressive),
                                 std::ref(tankFactoryCalm),
                                 std::ref(scoreAggressive),
                                 std::ref(scoreCalm),
                                 std::ref(scoreMutex));
            active++;
            if (active == (size_t)numThreads) {
                for (auto& t : threads) t.join();
                threads.clear();
                active = 0;
            }
        }
    }
    for (auto& t : threads) t.join();

    // Write results
    std::string filename = makeOutputFilename(mapsFolder);
    std::ofstream fout(filename);
    std::ostream& out = fout.is_open() ? fout : std::cout;
    if (!fout) {
        std::cerr << "Error: could not create " << filename << ", writing to stdout.\n";
    }

    out << "game_maps_folder=" << mapsFolder << "\n";
    out << "game_manager=GameManager\n\n";
    out << "AggressiveTank.so " << scoreAggressive << "\n";
    out << "CalmTank.so " << scoreCalm << "\n";

    std::cout << "\n=== Competition Results ===\n";
    std::cout << "AggressiveTank: " << scoreAggressive << "\n";
    std::cout << "CalmTank: " << scoreCalm << "\n";

    return 0;
}



// g++ -std=c++17 -I. -I./algorithm -I./common -I./GameManager Simulator/sim_competitive_demo.cpp GameManager/GameManager.cpp GameManager/Board.cpp GameManager/GameObject.cpp GameManager/utils.cpp GameManager/Vector2D.cpp GameManager/SatelliteViewImpl.cpp algorithm/MyPlayerFactory.cpp algorithm/algorithms.cpp algorithm/players/AbstractPlayer.cpp algorithm/players/AggressivePlayer.cpp algorithm/players/CalmPlayer.cpp algorithm/tanks/AbstractTankAlgorithm.cpp algorithm/tanks/AggressiveTank.cpp algorithm/tanks/CalmTank.cpp -o sim_competitive_demo
// ./sim_competitive_demo maps  
// ./sim_competitive_demo maps num_threads=4      