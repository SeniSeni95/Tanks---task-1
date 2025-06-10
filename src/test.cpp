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

    // Place a tank at (2,2)
    auto tank_ptr = std::make_shared<tank>('T', 0, 0, 0, 1, &board->arr[2][2], nullptr);
    tank_ptr->set_x(2);
    tank_ptr->set_y(2);
    tank_ptr->gear = "forward";
    board->arr[2][2].add_Object(tank_ptr);
    board->tanks.push_back(tank_ptr);

    // Debug: Check if (2,2) has objects
    std::cout << "Debug: cell (2,2) objects count: " << board->arr[2][2].objects.size() << std::endl;
    if (!board->arr[2][2].objects.empty()) {
        std::cout << "Debug: cell (2,2) first object symbol: " << board->arr[2][2].objects[0]->get_symbol() << std::endl;
    }

    // Place a shell at (1,2)
    auto shell_ptr = std::make_shared<shell>(&board->arr[1][2], 1, 0);
    board->arr[1][2].add_Object(shell_ptr);
    board->shells.push_back(shell_ptr);

    // Debug: Check if (1,2) has objects
    std::cout << "Debug: cell (1,2) objects count: " << board->arr[1][2].objects.size() << std::endl;
    if (!board->arr[1][2].objects.empty()) {
        std::cout << "Debug: cell (1,2) first object symbol: " << board->arr[1][2].objects[0]->get_symbol() << std::endl;
    }

    // SatelliteViewImpl setup
    SatelliteViewImpl satview;
    satview.updateCopy(*board);

    // Print the board state before simulation
    std::cout << "Board before simulate_step:\n";
    board->print_board();

    // Prepare simulate_step parameters
    std::vector<std::tuple<int, int, int, int>> shell_data = {
        std::make_tuple(1, 2, 1, 0) // shell at (1,2) moving down
    };
    std::vector<std::tuple<int, int, int, int, std::string>> tank_data = {
        std::make_tuple(2, 2, 0, 1, "forward") // tank at (2,2) facing right
    };

    // User input for action
    std::string action;
    std::cout << "Enter action for the tank (e.g., fw, bw, shoot, r4r, r4l, r8r, r8l): ";
    std::cin >> action;

    // Simulate a step
    auto new_satview = satview.simulate_step(shell_data, tank_data, action);

    // Print the board state after simulation
    std::cout << "Board after simulate_step:\n";
   new_satview->print_board();

    return 0;
}