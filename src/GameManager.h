#pragma once
#include <string>
#include <memory>
#include <vector>
#include <fstream>
#include <iostream>
#include "Board.h"
#include "PlayerFactory.h"
#include "TankAlgorithmFactory.h"
#include "SatelliteView.h"
#include "SatelliteViewImpl.h"

class GameManager {
public:
    GameManager(std::unique_ptr<PlayerFactory> playerFactory, std::unique_ptr<TankAlgorithmFactory> tankFactory);

    void readBoard(const std::string& filename);
    void run();// main game loop
    std::string actionToString(ActionRequest action);
    std::string askAlgorithm(tank* t);


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
    std::unique_ptr<SatelliteView> satview;
    std::vector<std::unique_ptr<TankAlgorithm>> algorithms;
    std::unique_ptr<PlayerFactory> playerFactory;
    std::unique_ptr<TankAlgorithmFactory> tankAlgorithmFactory;
    size_t maxSteps = 0;
    size_t numShells = 0;
     std::string input_filename;

    std::vector<std::shared_ptr<Player>> players;
    std::vector<std::shared_ptr<TankAlgorithm>> tankAlgorithms;
    std::vector<TankState> tankStates;
    SatelliteViewImpl satelliteViewImpl;
    bool satelliteCopyReady = false;
    
    

    size_t stepCounter = 0;
    size_t outOfShellSteps = 0;
    const size_t OUT_OF_SHELL_TIMEOUT = 40;

    bool allTanksOutOfShells() const;
    void applyActions(const std::vector<ActionRequest>& actions, std::ostream& output);
    void updateAliveStatus(); // for marking killed tanks
    void logRound(const std::vector<ActionRequest>& actions, std::ostream& output);
    void printGameResult(std::ostream& output);
};
