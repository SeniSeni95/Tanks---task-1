#include "GameManager.h"
#include "MyPlayerFactory.h"
#include "MyTankAlgorithmFactory.h"
#include <iostream>

int main(int argc, char** argv) {
    // std::cout << "[DEBUG] Program started." << std::endl;

    if (argc != 2) {
        std::cerr << "Usage: tanks_game <game_board_input_file>" << std::endl;
        return 1;
    }

    std::string inputFile = argv[1];
    // std::cout << "[DEBUG] Input file received: " << inputFile << std::endl;

    // std::cout << "[DEBUG] Creating GameManager..." << std::endl;
    GameManager game(
        std::make_unique<MyPlayerFactory>(),
        std::make_unique<MyTankAlgorithmFactory>()
    );

    // std::cout << "[DEBUG] Reading board from file..." << std::endl;
    game.readBoard(inputFile);

    // std::cout << "[DEBUG] Running game..." << std::endl;
    game.run();

    // std::cout << "[DEBUG] Program finished successfully." << std::endl;
    return 0;
}


 // chcp 65001
