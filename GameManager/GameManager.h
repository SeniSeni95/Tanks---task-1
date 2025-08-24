#pragma once
#include <string>
#include <memory>
#include <vector>
#include <fstream>
#include "Board.h"
#include "PlayerFactory.h"
#include "TankAlgorithmFactory.h"
#include "SatelliteView.h"
#include "SatelliteViewImpl.h"
#include "AbstractGameManager.h"
#include "GameResult.h"
#include "ActionRequest.h"

std::string actionToString(ActionRequest action);
ActionRequest stringToAction(const std::string& actionStr);

class GameManager : public AbstractGameManager {
public:
    GameManager(std::unique_ptr<PlayerFactory> playerFactory,
                std::unique_ptr<TankAlgorithmFactory> tankFactory,
                bool verbose);

    GameResult run(
        size_t map_width, size_t map_height,
        const SatelliteView& map,
        std::string map_name,
        size_t max_steps, size_t num_shells,
        Player& player1, std::string name1,
        Player& player2, std::string name2,
        TankAlgorithmFactory player1_tank_algo_factory,
        TankAlgorithmFactory player2_tank_algo_factory
    ) override;

private:
    std::unique_ptr<game_board> board;
    std::unique_ptr<SatelliteView> satview; // for updates during turns
    bool satelliteCopyReady = false;

    std::unique_ptr<PlayerFactory> playerFactory;
    std::unique_ptr<TankAlgorithmFactory> tankAlgorithmFactory;

    bool verboseOutput = false;
    std::ofstream verboseFile;
};
