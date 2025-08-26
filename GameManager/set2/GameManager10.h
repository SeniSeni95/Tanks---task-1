#pragma once
#include <string>
#include <memory>
#include <vector>
#include <fstream>
#include <map>
#include "Board.h"
#include "../common/ActionUtils.h"
#include "../common/PlayerFactory.h"
#include "../common/SatelliteView.h"
#include "SatelliteViewImpl.h"
#include "../common/AbstractGameManager.h"
#include "../common/GameResult.h"
#include "../common/ActionRequest.h"
#include "../algorithm/MyTankAlgorithmFactory.h"

class GameManager10 : public AbstractGameManager {
public:
    GameManager10(PlayerFactory playerFactory,
                 MyTankAlgorithmFactory tankFactory,
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
    std::vector<std::unique_ptr<TankAlgorithm>> tankAlgorithms;
    std::string commandStringToEnumName(const std::string& cmd);
    std::string actionToString(ActionRequest action);
    ActionRequest stringToAction(const std::string& actionStr);

    std::unique_ptr<game_board> board;
    std::unique_ptr<SatelliteView> satview; // for updates during turns
    bool satelliteCopyReady = false;

    PlayerFactory playerFactory;
    MyTankAlgorithmFactory myTankAlgorithmFactory;

    bool verboseOutput = false;
    std::ofstream verboseFile;

    // GM2 specific: max turns per tank tracking
    static const int MAX_TURNS_PER_TANK = 5;
    std::map<tank*, int> tank_turn_counters;
};
