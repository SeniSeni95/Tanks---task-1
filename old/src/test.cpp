#include "SatelliteViewImpl.h"
#include "Board.h"
#include <iostream>
#include <vector>
#include <tuple>
#include <memory>

static_assert(sizeof(SatelliteViewImpl) > 0, "SatelliteViewImpl is visible");

int main() {
    int rows = 5, cols = 5;

    // Create base board (could be empty)
    std::vector<std::vector<cell>> arr;
    arr.reserve(rows);
    for (int i = 0; i < rows; ++i) {
        std::vector<cell> row;
        row.reserve(cols);
        for (int j = 0; j < cols; ++j) {
            row.emplace_back(i, j);
        }
        arr.push_back(std::move(row));
    }
    auto base_board = std::make_unique<game_board>(rows, cols, std::move(arr));

    // Place wall and mine for visual diversity (optional)
    base_board->arr[0][0].add_Object(std::make_shared<wall>('#', &base_board->arr[0][0]));
    base_board->arr[4][4].add_Object(std::make_shared<mine>('@', &base_board->arr[4][4]));

    // Tank and shell data
    std::vector<std::tuple<int, int, int, int>> shell_data = {
        std::make_tuple(1, 2, 1, 0) // shell at (1,2) moving down
    };
    std::vector<std::tuple<int, int, int, int, std::string>> tank_data = {
        std::make_tuple(2, 2, 0, 1, "forward") // tank at (2,2) facing right
    };

    // Set up SatelliteView with a copy of the board
    SatelliteViewImpl satview;
    satview.updateCopy(*base_board);

    // Generate new board using SatelliteView + data
    auto sim_board = game_board::generate_board(satview, rows, cols, shell_data, tank_data);

    // Display board before step
    std::cout << "Board before simulate_step:\n";
    sim_board->print_board();

    // Get action from user
    std::string action;
    std::cout << "Enter action for the tank (e.g., fw, bw, shoot, r4r, r4l, r8r, r8l): ";
    std::cin >> action;

    // Simulate
    sim_board->simulate_step(std::make_tuple(2, 2, action));

    // Display board after step
    std::cout << "Board after simulate_step:\n";
    sim_board->print_board();

    return 0;
}
