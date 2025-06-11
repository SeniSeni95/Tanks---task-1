#include "SatelliteViewImpl.h"
#include "Board.h"
#include <iostream>
#include <vector>
#include <tuple>
#include <memory>

static_assert(sizeof(SatelliteViewImpl) > 0, "SatelliteViewImpl is visible");

int main() {
    int rows = 5, cols = 5;
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
    auto board = std::make_unique<game_board>(rows, cols, std::move(arr));

    // Prepare shell and tank data
    std::vector<std::tuple<int, int, int, int>> shell_data = {
        std::make_tuple(1, 2, 1, 0) // shell at (1,2) moving down
    };
    std::vector<std::tuple<int, int, int, int, std::string>> tank_data = {
        std::make_tuple(2, 2, 0, 1, "forward") // tank at (2,2) facing right
    };

    // Generate the board with the objects
    auto sim_board = SatelliteViewImpl::generate_board(*board, shell_data, tank_data);

    // Print the board state before simulation
    std::cout << "Board before simulate_step:\n";
    sim_board->print_board();

    // User input for action
    std::string action;
    std::cout << "Enter action for the tank (e.g., fw, bw, shoot, r4r, r4l, r8r, r8l): ";
    std::cin >> action;

    // Simulate a step for the tank at (2,2)
    SatelliteViewImpl::simulate_step(*sim_board, std::make_tuple(2, 2, action));

    // Print the board state after simulation
    std::cout << "Board after simulate_step:\n";
    sim_board->print_board();

    return 0;
}