#pragma once
#include <string>
#include <memory>
#include <vector>
#include <fstream>
#include <iostream>
#include "Board.h"
#include "PlayerFactory.h"
#include "TankAlgorithmFactory.h"

class GameManager {
public:
    GameManager(const PlayerFactory& playerFactory, const TankAlgorithmFactory& tankFactory);

    bool readBoard(const std::string& filename);
    void run(); // main game loop

private:
    struct TankState {
        int playerId;
        int tankIndex;
        bool alive;
        size_t x, y;
        int shellsRemaining;
        int cooldown;
        std::string lastAction;
    };

    std::unique_ptr<game_board> board;
    const PlayerFactory& playerFactory;
    const TankAlgorithmFactory& tankFactory;
    size_t maxSteps = 0;
    size_t numShells = 0;
    std::string inputFilename;

    std::vector<std::shared_ptr<Player>> players;
    std::vector<std::shared_ptr<TankAlgorithm>> tankAlgorithms;
    std::vector<TankState> tankStates;

    size_t stepCounter = 0;
    size_t outOfShellSteps = 0;
    const size_t OUT_OF_SHELL_TIMEOUT = 40;

    bool allTanksOutOfShells() const;
    void applyActions(const std::vector<ActionRequest>& actions, std::ostream& output);
    void updateAliveStatus(); // for marking killed tanks
    void logRound(const std::vector<ActionRequest>& actions, std::ostream& output);
    void printGameResult(std::ostream& output);
};
