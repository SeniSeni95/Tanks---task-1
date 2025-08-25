#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <set>
#include <stdexcept>

#include "../algorithm/tanks/AggressiveTank.h"
#include "../algorithm/tanks/CalmTank.h"
#include "MyPlayerFactory.h"
#include "MyTankAlgorithmFactory.h"

#include "GameManager.h"

#include "GameResult.h"
#include "SatelliteViewImpl.h"
#include "Player.h"
#include "Board.h"
#include "GameObject.h"

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
LoadedMap buildStaticMap() {
    int cols = 8, rows = 8;
    int maxSteps = 50, numShells = 20;

    if (DEBUG_ENABLED) {
        std::cout << "[DEBUG] Creating base board...\n";
    }

    // 1. create cells
    std::vector<std::vector<cell>> arr;
    arr.reserve(cols);
    for (int i = 0; i < cols; ++i) {
        std::vector<cell> row;
        row.reserve(rows);
        for (int j = 0; j < rows; ++j) {
            row.emplace_back(i, j);
        }
        arr.push_back(std::move(row));
    }

    // 2. make the board
    auto board = std::make_unique<game_board>(cols, rows, std::move(arr));

    // Player 1: 5 tanks on top row (symbols only, no algos yet)
    for (int k = 0; k < 5; ++k) {
        cell& c = board->arr[k][0];
        auto t = std::make_shared<tank>('1', 1, k, 0, 1, &c, nullptr);
        c.add_Object(t);
        board->tanks.push_back(t);
    }

    // Player 2: 2 tanks on bottom row
    for (int k = 0; k < 2; ++k) {
        cell& c = board->arr[7 - k][7];
        auto t = std::make_shared<tank>('2', 2, k, 0, -1, &c, nullptr);
        c.add_Object(t);
        board->tanks.push_back(t);
    }

    // 3. add walls
    board->arr[3][3].add_Object(std::make_shared<wall>('#', &board->arr[3][3]));
    board->arr[4][4].add_Object(std::make_shared<wall>('#', &board->arr[4][4]));

    // 4. add mines
    board->arr[2][5].add_Object(std::make_shared<mine>('@', &board->arr[2][5]));
    board->arr[5][2].add_Object(std::make_shared<mine>('@', &board->arr[5][2]));

    // 5. create players (for metadata)
    MyPlayerFactory pf;
    auto p1 = pf.create(1, cols, rows, maxSteps, numShells);
    auto p2 = pf.create(2, cols, rows, maxSteps, numShells);

    // 6. prepare result
    LoadedMap lm;
    lm.width = cols;
    lm.height = rows;
    lm.maxSteps = maxSteps;
    lm.numShells = numShells;
    lm.view.updateCopy(*board);   // SatelliteView holds only the layout
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
// main
//----------------------------------
int main() {
    std::cout << "=== DEMO SIMULATOR (Static 8x8 Map) ===\n";

    MyPlayerFactory pf;
    PlayerFactory playerFactory = [&pf](int p, size_t x, size_t y, size_t maxS, size_t numS) {
        if (DEBUG_ENABLED) {
            std::cout << "[DEBUG] Creating Player " << p << "\n";
        }
        return pf.create(p, x, y, maxS, numS);
    };

    // Tank algorithm factory: Player 1 uses Aggressive, Player 2 uses Calm
    MyTankAlgorithmFactory tankFactory = [](int player, int tank) -> std::unique_ptr<TankAlgorithm> {
        if (DEBUG_ENABLED) {
            std::cout << "[DEBUG] Creating TankAlgorithm for Player " << player 
                      << ", Tank " << tank << "\n";
        }
        if (player == 1) {
            return std::make_unique<AggressiveTank>(player, tank);
        } else {
            return std::make_unique<CalmTank>(player, tank);
        }
    };

    // Build static map (pass the tank factory in)
    LoadedMap map = buildStaticMap();
    if (DEBUG_ENABLED) {
        std::cout << "[DEBUG] Map built: " 
                  << map.width << "x" << map.height 
                  << " with " << map.board->tanks.size() << " tanks.\n";
    }

    // Create GameManager with factories
    if (DEBUG_ENABLED) {
        std::cout << "[DEBUG] Initial board from SatelliteView:\n";
        map.view.print_board();
    }
    GameManager manager(playerFactory, tankFactory, false);

    // Run game
    if (DEBUG_ENABLED) {
        std::cout << "[DEBUG] Running game...\n";
    }
    GameResult res = manager.run(
        map.width, map.height, map.view, "static_map",
        map.maxSteps, map.numShells,
        *map.p1, "P1", *map.p2, "P2",
        tankFactory, tankFactory
    );

    if (DEBUG_ENABLED) {
        std::cout << "[DEBUG] Game finished.\n";
    }
    std::cout << "Winner: " << winnerToString(res.winner)
              << " | Reason: " << reasonToString(res.reason)
              << " | Rounds: " << res.rounds << "\n";

    return 0;
}



 // chcp 65001