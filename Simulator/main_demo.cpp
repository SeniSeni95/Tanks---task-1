#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <set>
#include <stdexcept>

#include "MyPlayerFactory.h"
#include "MyTankAlgorithmFactory.h"

#include "GameManager.h"

#include "GameResult.h"
#include "SatelliteViewImpl.h"
#include "Player.h"
#include "Board.h"
#include "GameObject.h"

// A struct for passing map info around
struct LoadedMap {
    SatelliteViewImpl view;
    size_t width{};
    size_t height{};
    size_t maxSteps{};
    size_t numShells{};
};

// Build a static 8x8 map with 5 tanks for P1, 2 tanks for P2, 2 walls, 2 mines
LoadedMap buildStaticMap() {
    int cols = 8, rows = 8;
    int maxSteps = 50, numShells = 20;

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
    MyTankAlgorithmFactory algoFactory;

    // Player 1: 5 tanks on top row
    for (int k = 0; k < 5; ++k) {
        cell& c = board->arr[k][0];
        auto algo = algoFactory(1, k);
        auto t = std::make_shared<tank>('1', 1, k, 0, 1, &c, algo.release());
        c.add_Object(t);
        board->tanks.push_back(t);
    }

    // Player 2: 2 tanks on bottom row
    for (int k = 0; k < 2; ++k) {
        cell& c = board->arr[7 - k][7];
        auto algo = algoFactory(2, k);
        auto t = std::make_shared<tank>('2', 2, k, 0, -1, &c, algo.release());
        c.add_Object(t);
        board->tanks.push_back(t);
    }

    // 3. add walls
    board->arr[3][3].add_Object(std::make_shared<wall>('#', &board->arr[3][3]));
    board->arr[4][4].add_Object(std::make_shared<wall>('#', &board->arr[4][4]));

    // 4. add mines
    board->arr[2][5].add_Object(std::make_shared<mine>('@', &board->arr[2][5]));
    board->arr[5][2].add_Object(std::make_shared<mine>('@', &board->arr[5][2]));

    // 5. prepare result
    LoadedMap lm;
    lm.width = cols;
    lm.height = rows;
    lm.maxSteps = maxSteps;
    lm.numShells = numShells;
    lm.view.updateCopy(*board);
    return lm;
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

int main() {
    std::cout << "=== DEMO SIMULATOR (Static 8x8 Map) ===\n";

    // Build static map
    LoadedMap map = buildStaticMap();

    // Create players
    MyPlayerFactory pf;
    auto p1 = pf.create(1, map.width, map.height, map.maxSteps, map.numShells);
    auto p2 = pf.create(2, map.width, map.height, map.maxSteps, map.numShells);

    // GameManager with dummy PlayerFactory (not used in static map)
    GameManager manager(
        [](int, size_t, size_t, size_t, size_t) {
            return std::unique_ptr<Player>(nullptr);  // not used
        },
        MyTankAlgorithmFactory(),
        false);

    // Run game
    GameResult res = manager.run(
        map.width, map.height, map.view, "static_map",
        map.maxSteps, map.numShells, *p1, "P1", *p2, "P2",
        MyTankAlgorithmFactory(), MyTankAlgorithmFactory()
    );

    std::cout << "Winner: " << winnerToString(res.winner)
              << " | Reason: " << reasonToString(res.reason)
              << " | Rounds: " << res.rounds << "\n";

    return 0;
}
