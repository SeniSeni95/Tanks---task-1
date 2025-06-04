#include "GameManager.h"
#include "MyPlayerFactory.h"
#include "MyTankAlgorithmFactory.h"

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: tanks_game <game_board_input_file>" << std::endl;
        return 1;
    }

    GameManager game(MyPlayerFactory{}, MyTankAlgorithmFactory{});
    if (!game.readBoard(argv[1])) {
        std::cerr << "Failed to load board!" << std::endl;
        return 1;
    }

    game.run();  // simulate rounds + output file
    return 0;
}