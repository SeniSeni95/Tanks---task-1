#include <iostream>
#include <memory>
#include <chrono>
#include <iomanip>
#include <vector>
#include <thread>
#include <mutex>
#include <string>
#include <fstream>
#include <sstream>
#include <set>
#include <stdexcept>
#include <filesystem>
#include <algorithm>  // for std::equal
#include <iostream>   // optional, for debug

#include "../algorithm/tanks/AggressiveTank.h"
#include "../algorithm/tanks/CalmTank.h"
#include "MyPlayerFactory.h"
#include "MyTankAlgorithmFactory.h"

// Set 1: GameManager 1, 4, 8, 9
#include "../GameManager/set1/GameManager1.h"
#include "../GameManager/set1/GameManager4.h"
#include "../GameManager/set1/GameManager8.h"
#include "../GameManager/set1/GameManager9.h"

// Set 2: GameManager 2, 10
#include "../GameManager/set2/GameManager2.h"
#include "../GameManager/set2/GameManager10.h"

// Set 3: GameManager 3, 5, 6, 7, 11, 12
#include "../GameManager/set3/GameManager3.h"
#include "../GameManager/set3/GameManager5.h"
#include "../GameManager/set3/GameManager6.h"
#include "../GameManager/set3/GameManager7.h"
#include "../GameManager/set3/GameManager11.h"
#include "../GameManager/set3/GameManager12.h"

#include "GameResult.h"
#include "SatelliteViewImpl.h"
#include "Player.h"
#include "Board.h"
#include "GameObject.h"
#include "../common/AbstractGameManager.h"

// Debug control - set to true to enable debugging, false to disable
static const bool DEBUG_ENABLED = false;

//----------------------------------
// A struct for passing map info around
//----------------------------------
struct LoadedMap {
    SatelliteViewImpl view;
    std::unique_ptr<game_board> board;
    std::vector<std::unique_ptr<TankAlgorithm>> tankAlgorithms;
    std::unique_ptr<Player> p1;
    std::unique_ptr<Player> p2;
    size_t width{};
    size_t height{};
    size_t maxSteps{};
    size_t numShells{};
};

//----------------------------------
// Build a static 8x8 demo board
//----------------------------------
LoadedMap buildMapFromFile(const std::string& filename) {
    LoadedMap lm;
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cerr << "[ERROR] Failed to open map file: " << filename << std::endl;
        return lm;
    }

    std::string line;
    getline(file, line); // Skip map description line like "1-input a"

    int maxSteps = 0, numShells = 0, rows = 0, cols = 0;
    getline(file, line); sscanf(line.c_str(), "MaxSteps = %d", &maxSteps);
    getline(file, line); sscanf(line.c_str(), "NumShells = %d", &numShells);
    getline(file, line); sscanf(line.c_str(), "Rows = %d", &rows);
    getline(file, line); sscanf(line.c_str(), "Cols = %d", &cols);

    // 1. create cells
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

    // 2. make the board
    auto board = std::make_unique<game_board>(cols, rows, std::move(arr));

    std::set<char> valid_chars = {'#', '1', '2', ' ', '@'};
    std::vector<int> tank_counters(2, 0); // index 0 for P1, 1 for P2

    // 3. parse rows of the map
    for (int j = 0; j < rows; ++j) {
        getline(file, line);
        for (int i = 0; i < cols; ++i) {
            char ch = (i < static_cast<int>(line.size())) ? line[i] : ' ';
            if (valid_chars.find(ch) == valid_chars.end()) ch = ' ';

            cell& current = board->get_cell(i, j);

            if (ch == '#') {
                current.add_Object(std::make_shared<wall>('#', &current));
            } else if (ch == '@') {
                current.add_Object(std::make_shared<mine>('@', &current));
            } else if (ch == '1' || ch == '2') {
                int player_number = (ch == '1') ? 0 : 1;
                int tank_number = tank_counters[player_number]++;
                int directionx = (player_number == 0) ? -1 : 1;
                auto tank_ptr = std::make_shared<tank>(
                    ch, player_number + 1, tank_number,
                    directionx, 0, &current, nullptr
                );
                board->tanks.push_back(tank_ptr);
                current.add_Object(tank_ptr);
            }
        }
    }

    file.close();

    // 4. create players (metadata)
    MyPlayerFactory pf;
    auto p1 = pf.create(1, cols, rows, maxSteps, numShells);
    auto p2 = pf.create(2, cols, rows, maxSteps, numShells);

    // 5. prepare LoadedMap
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
// Helpers for printing results
//----------------------------------
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
// Step 2: Scan folder for GM numbers
//----------------------------------
namespace fs = std::filesystem;

std::vector<int> detectGameManagers(const std::string& folder) {
    std::vector<int> found;
    for (auto& entry : fs::directory_iterator(folder)) {
        if (!entry.is_regular_file()) continue;
        auto path = entry.path();
        if (path.extension() != ".cpp") continue;  // only count .cpp files

        std::string name = path.filename().string();
        if (name.rfind("GameManager", 0) == 0) { // starts with "GameManager"
            std::string numStr = name.substr(std::string("GameManager").size());
            numStr = numStr.substr(0, numStr.find('.')); // cut extension
            try {
                int num = std::stoi(numStr);
                found.push_back(num);
            } catch (...) {
                continue;
            }
        }
    }
    return found;
}


//----------------------------------
// Step 3: Build vector<AbstractGameManager>
//----------------------------------
std::vector<std::unique_ptr<AbstractGameManager>>
buildManagers(const std::vector<int>& gmNums,
              PlayerFactory& playerFactory,
              MyTankAlgorithmFactory& tankFactory) 
{
    std::vector<std::unique_ptr<AbstractGameManager>> managers;
    for (int gmNum : gmNums) {
        switch (gmNum) {
            case 1: managers.push_back(std::make_unique<GameManager1>(playerFactory, tankFactory, false)); break;
            case 2: managers.push_back(std::make_unique<GameManager2>(playerFactory, tankFactory, false)); break;
            case 3: managers.push_back(std::make_unique<GameManager3>(playerFactory, tankFactory, false)); break;
            case 4: managers.push_back(std::make_unique<GameManager4>(playerFactory, tankFactory, false)); break;
            case 5: managers.push_back(std::make_unique<GameManager5>(playerFactory, tankFactory, false)); break;
            case 6: managers.push_back(std::make_unique<GameManager6>(playerFactory, tankFactory, false)); break;
            case 7: managers.push_back(std::make_unique<GameManager7>(playerFactory, tankFactory, false)); break;
            case 8: managers.push_back(std::make_unique<GameManager8>(playerFactory, tankFactory, false)); break;
            case 9: managers.push_back(std::make_unique<GameManager9>(playerFactory, tankFactory, false)); break;
            case 10: managers.push_back(std::make_unique<GameManager10>(playerFactory, tankFactory, false)); break;
            case 11: managers.push_back(std::make_unique<GameManager11>(playerFactory, tankFactory, false)); break;
            case 12: managers.push_back(std::make_unique<GameManager12>(playerFactory, tankFactory, false)); break;
            default:
                std::cerr << "Unknown GameManager number: " << gmNum << "\n";
        }
    }
    return managers;
}


bool equalGameResults(const GameResult& a, const GameResult& b,
                      size_t width, size_t height) {
    // 1. Compare basic fields
    if (a.winner != b.winner) return false;
    if (a.reason != b.reason) return false;
    if (a.rounds != b.rounds) return false;

    // 2. Compare remaining tanks
    if (a.remaining_tanks.size() != b.remaining_tanks.size()) return false;
    if (!std::equal(a.remaining_tanks.begin(), a.remaining_tanks.end(),
                    b.remaining_tanks.begin())) {
        return false;
    }

    // 3. Compare final board state
    if (!a.gameState || !b.gameState) {
        return a.gameState == nullptr && b.gameState == nullptr;
    }

    for (size_t x = 0; x < width; x++) {
        for (size_t y = 0; y < height; y++) {
            char ca = a.gameState->getObjectAt(x, y);
            char cb = b.gameState->getObjectAt(x, y);
            if (ca != cb) {
                return false;
            }
        }
    }

    return true;
}


//----------------------------------
// helper: make output filename
//----------------------------------
std::string makeOutputFilename(const std::string& folder) {
    auto now = std::chrono::system_clock::now();
    auto t = std::chrono::system_clock::to_time_t(now);

    std::stringstream ss;
    ss << folder << "/comparative_results_" << t << ".txt";
    return ss.str();
}


//----------------------------------
// Result grouping structure
//----------------------------------
struct ResultGroup {
    GameResult result;          // representative result
    std::vector<int> gmNumbers; // GM numbers that produced it
};

//----------------------------------
// helper: write one group
//----------------------------------
void writeGroup(std::ostream& out, const ResultGroup& g, size_t width, size_t height) {
    // a) GM list
    for (size_t j = 0; j < g.gmNumbers.size(); j++) {
        if (j > 0) out << ",";
        out << "GM" << g.gmNumbers[j];
    }
    out << "\n";

    // b) Result message
    if (g.result.winner == 0) {
        out << "Tie - " << reasonToString(g.result.reason) << "\n";
    } else {
        out << "Player " << g.result.winner << " won - " << reasonToString(g.result.reason) << "\n";
    }

    // c) Round number
    out << g.result.rounds << "\n";

    // d) Final map
    if (g.result.gameState) {
        for (size_t y = 0; y < height; y++) {
            for (size_t x = 0; x < width; x++) {
                out << g.result.gameState->getObjectAt(x, y);
            }
            out << "\n";
        }
    }
}

//----------------------------------
// main
//----------------------------------
int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <folder> <mapfile>\n";
        return 1;
    }
    std::string folder  = argv[1];
    std::string mapfile = argv[2];

    std::cout << "=== DEMO SIMULATOR (Comparative Run, 4 threads) ===\n";

    // Factories
    MyPlayerFactory pf;
    PlayerFactory playerFactory = [&pf](int p, size_t x, size_t y, size_t maxS, size_t numS) {
        return pf.create(p, x, y, maxS, numS);
    };
    MyTankAlgorithmFactory tankFactory = [](int player, int tank) -> std::unique_ptr<TankAlgorithm> {
        if (player == 1) {
            return std::make_unique<AggressiveTank>(player, tank);
        } else {
            return std::make_unique<CalmTank>(player, tank);
        }
    };

    // Detect GM's
    std::vector<int> gmNums = detectGameManagers(folder);
    if (gmNums.empty()) {
        std::cout << "No GameManagers found in " << folder << "\n";
        return 0;
    }

    // Build managers
    auto managers = buildManagers(gmNums, playerFactory, tankFactory);

    std::vector<ResultGroup> groups;
    size_t width = 0, height = 0;

    // --- Parallel execution ---
    const int num_threads = 4; // fixed thread count
    std::vector<GameResult> allResults(managers.size());

    auto worker = [&](size_t idx) {
        LoadedMap map = buildMapFromFile(mapfile);
        width  = map.width;
        height = map.height;

        GameResult res = managers[idx]->run(
            map.width, map.height, map.view, mapfile,
            map.maxSteps, map.numShells,
            *map.p1, "P1", *map.p2, "P2",
            tankFactory, tankFactory
        );

        allResults[idx] = std::move(res);
    };

    std::vector<std::thread> threads;
    size_t active = 0;
    for (size_t i = 0; i < managers.size(); i++) {
        threads.emplace_back(worker, i);
        active++;
        if (active == (size_t)num_threads) {
            for (auto& t : threads) t.join();
            threads.clear();
            active = 0;
        }
    }
    for (auto& t : threads) t.join();

    // --- Group results ---
    for (size_t i = 0; i < managers.size(); i++) {
        GameResult& res = allResults[i];

        std::cout << "[GameManager" << gmNums[i] << "] Winner: "
                  << winnerToString(res.winner)
                  << " | Reason: " << reasonToString(res.reason)
                  << " | Rounds: " << res.rounds << "\n";

        bool found = false;
        for (auto& g : groups) {
            if (equalGameResults(res, g.result, width, height)) {
                g.gmNumbers.push_back(gmNums[i]);
                found = true;
                break;
            }
        }
        if (!found) {
            ResultGroup g{std::move(res), {gmNums[i]}};
            groups.push_back(std::move(g));
        }
    }

    // === Write comparative output file ===
    std::string filename = makeOutputFilename(folder);
    std::ofstream fout(filename);
    std::ostream& out = fout.is_open() ? fout : std::cout;
    if (!fout) {
        std::cerr << "Error: could not create " << filename
                  << ", writing to stdout instead.\n";
    }

    // Header lines
    out << "game_map=" << mapfile << "\n";
    out << "algorithm1=AggressiveTank.so\n";  // placeholder
    out << "algorithm2=CalmTank.so\n";        // placeholder
    out << "\n";

    // Write all groups
    for (size_t idx = 0; idx < groups.size(); idx++) {
        writeGroup(out, groups[idx], width, height);
        if (idx + 1 < groups.size()) out << "\n"; // blank line between groups
    }

    // Also echo grouping to screen for clarity
    std::cout << "\n=== Grouped Results ===\n";
    for (size_t idx = 0; idx < groups.size(); idx++) {
        std::cout << "Result " << (idx + 1) << ": ";
        for (size_t j = 0; j < groups[idx].gmNumbers.size(); j++) {
            if (j > 0) std::cout << ", ";
            std::cout << "GM" << groups[idx].gmNumbers[j];
        }
        std::cout << " | Winner: " << winnerToString(groups[idx].result.winner)
                  << " | Reason: " << reasonToString(groups[idx].result.reason)
                  << " | Rounds: " << groups[idx].result.rounds
                  << "\n";
    }

    return 0;
}


// g++ -std=c++17 -I. -I./algorithm -I./common -I./GameManager Simulator/sim_comparative_demo.cpp GameManager/set1/GameManager1.cpp GameManager/set1/GameManager4.cpp GameManager/set1/GameManager8.cpp GameManager/set1/GameManager9.cpp GameManager/set2/GameManager2.cpp GameManager/set2/GameManager10.cpp GameManager/set3/GameManager3.cpp GameManager/set3/GameManager5.cpp GameManager/set3/GameManager6.cpp GameManager/set3/GameManager7.cpp GameManager/set3/GameManager11.cpp GameManager/set3/GameManager12.cpp GameManager/Board.cpp GameManager/GameObject.cpp GameManager/utils.cpp GameManager/Vector2D.cpp GameManager/SatelliteViewImpl.cpp algorithm/MyPlayerFactory.cpp algorithm/algorithms.cpp algorithm/players/AbstractPlayer.cpp algorithm/players/AggressivePlayer.cpp algorithm/players/CalmPlayer.cpp algorithm/tanks/AbstractTankAlgorithm.cpp algorithm/tanks/AggressiveTank.cpp algorithm/tanks/CalmTank.cpp -o sim_comparative_demo
//./sim_comparative_demo GameManager/set1 maps/map1.txt
// set1/2/3, map 1/2/3