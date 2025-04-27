#include <iostream>
#include <vector>
#include <string>
#include <math.h>
#include <algorithm>
#include <unistd.h>
#include "board.h"
#include "game_objects.h"
#include "algorithms.h"
#include <set>
#include <fstream>
#include <sstream>
using namespace std;
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Forward declarations
struct game_object;
struct tank;
struct shell;
struct mine;
struct wall;
struct game_board;
struct cell;
struct game_master;

// Constants
const int LOOKAHEAD_DEPTH = 2; // Lookahead depth for the algorithms

struct game_master{
    game_board* board;
    bool game_over;
    bool out_of_shells;
    int time_out_steps;

    algorithm* algo1;
    algorithm* algo2;

    game_master(game_board* board, algorithm* algo1, algorithm* algo2) {
        this->board = board; // Initialize the game board
        this->game_over = false; // Set game_over to false initially
        this->time_out_steps=40;
        this->out_of_shells=false;

        this->algo1 = algo1; // Initialize the first algorithm
        this->algo2 = algo2; // Initialize the second algorithm
    }
    void start_game() {
        // Function to start the game
        cout << "Game started!" << endl;
        // Add game logic here
    }
    #include <fstream>
#include <iostream>

void game() {
    // Create the output file to log the game moves
    std::ofstream game_output("game_moves.txt");

    // Check if the file was opened successfully
    if (!game_output.is_open()) {
        std::cerr << "Error opening game log file!" << std::endl;
        return;
    }

    while ((time_out_steps >= 0) && !game_over) {
        // Check if all tanks are out of shells
        out_of_shells = true; // Assume all tanks are out of shells
        for (tank* t : board->tanks) {
            if (t->shells > 0) {
                out_of_shells = false; // If any tank has shells, set to false
                break;
            }
        }

        // If out_of_shells is true, reduce time_out_steps
        if (out_of_shells) {
            time_out_steps--;
            cout << "All tanks are out of shells! Reducing time_out_steps to " << time_out_steps << endl;
        }

        // Reduce the shoot timer for each tank if it's greater than 0
        for (tank* t : board->tanks) {
            if (t->shot_timer > 0) {
                t->shot_timer--;
                cout << "Tank " << t->symbol << "'s shot timer reduced to " << t->shot_timer << endl;
            }
        }

        string move1 = ask_algorithm('1'); // Get the move for player 1
        string move2 = ask_algorithm('2'); // Get the move for player 2

        // Log the moves to the file
        game_output << "Player 1 Move: " << move1 << std::endl;
        game_output << "Player 2 Move: " << move2 << std::endl;

        bool t1 = turn('1', move1); // Call the turn function with player '1'
        bool t2 = turn('2', move2); // Call the turn function with player '2'
        game_over = board->handle_cell_collisions(); // Handle cell collisions

        if (game_over) {
            break; // Exit the loop if game is over
        }

        game_over = board->do_step(); // Perform half step again

        if (game_over) {
            break; // Exit the loop if game is over
        }

        sleep(1);
    }

    cout << "Game over!" << endl; // Print game over message

    // Log the winner or tie to the file
    if (board->tanks.size() == 1) {
        if (board->tanks[0]->symbol == '1') {
            game_output << "Player 1 wins!" << std::endl; // Player 1 wins
        } else {
            game_output << "Player 2 wins!" << std::endl; // Player 2 wins
        }
    } else if (board->tanks.size() == 0) {
        game_output << "It's a tie!" << std::endl; // It's a tie
    } else {
        game_output << "It's a tie!" << std::endl; // Game over
    }

    // Close the file
    game_output.close();
}
    
    
    bool turn(char player, string move) {    
        for (tank* t : board->tanks) {
            if (t->symbol == player) {
                t->turn(board, move);
                break;
            }
        }
        return false; // Return false if no valid tank was found
    }
  
    string ask_algorithm(char player) {
        // // Function to ask the player for their move
        // string move;
        // cout << "Enter your move (forward/backward/rotate left/rotate right/shoot): ";
        // cin >> move; // Get the move from the player
        // return move; // Return the move

        // Use the algorithm to decide the move
        cout << "Asking algorithm for player " << player << "'s move..." << endl;

        for (tank* t : board->tanks) {
            if (t->symbol == player) {
                std::pair<std::string, double> result;

                if (player == '1') {
                    result = algo1->decide_move(board, t, LOOKAHEAD_DEPTH); // Get the move from the algorithm
                } else if (player == '2') {
                    result = algo2->decide_move(board, t, LOOKAHEAD_DEPTH); // Get the move from the algorithm
                }

                cout << "Algorithm returned move: " << result.first << " with score: " << result.second << endl;

                return result.first; // Return the move
            }
        }

        return "skip";
    }
};


void export_errors(const std::string& filename, const std::vector<std::string>& errors) {
    std::ofstream error_file(filename);
    if (error_file.is_open()) {
        for (const auto& error : errors) {
            error_file << error << std::endl;
        }
    } else {
        std::cerr << "Error opening error file!" << std::endl;
    }
}

int main(int argc, char* argv[]) {
    std::cout << "v2" << std::endl;
    if (argc != 2) {
        std::cerr << "Usage: tanks_game <input_file>" << std::endl;
        return 1;
    }

    std::ifstream file(argv[1]);
    if (!file.is_open()) {
        std::cerr << "Error opening file!" << std::endl;
        return 1;
    }

    int n, m;
    file >> m >> n;  // Read dimensions (m = columns, n = rows)
    file.ignore();    // Ignore the newline character after reading dimensions
    file.ignore();    // Ignore the first empty line

    std::vector<std::vector<char>> boardchar(n, std::vector<char>(m));
    std::vector<std::string> errors;
    int count_1 = 0, count_2 = 0;

    // Allowed characters for the board
    const std::set<char> valid_chars = {'#', '1', '2', ' ', '@'};

    // Read the board from the file, starting from the second line
for (int i = 0; i < n; ++i) {
    std::string line;
    std::getline(file, line);

    for (int j = 0; j < m; ++j) {
        // Apply the cast here to avoid signed/unsigned comparison warning
        char ch = (static_cast<std::string::size_type>(j) < line.size()) ? line[j] : ' '; // Handle lines shorter than m
    
        if (valid_chars.find(ch) == valid_chars.end()) {
            ch = ' '; // Invalid character, turn into space
            errors.push_back("Invalid character found at " + std::to_string(i + 1) + ", " + std::to_string(j + 1));
        }
    
        // Check if we have more than 1 occurrence of '1' or '2'
        if (ch == '1') {
            if (count_1 > 0) {
                ch = ' '; // Replace with space if more than one '1' is found
                errors.push_back("More than one '1' found at " + std::to_string(i + 1) + ", " + std::to_string(j + 1));
            }
            count_1++;
        }
        if (ch == '2') {
            if (count_2 > 0) {
                ch = ' '; // Replace with space if more than one '2' is found
                errors.push_back("More than one '2' found at " + std::to_string(i + 1) + ", " + std::to_string(j + 1));
            }
            count_2++;
        }
    
        boardchar[i][j] = ch;
    }
}
    // Check for missing '1' or '2'
    if (count_1 == 0 || count_2 == 0) {
        errors.push_back("Fatal error: Missing '1' or '2'.");
        export_errors(std::string(argv[1]) + ".errors", errors);
        return 1;
    }

// Adjust the size of the board if necessary
if (boardchar.size() < static_cast<size_t>(n)) {
    // Add rows to match the required number of rows (pad with spaces)
    for (size_t i = boardchar.size(); i < static_cast<size_t>(n); ++i) {
        boardchar.push_back(std::vector<char>(m, ' ')); // Pad rows
    }
} else if (boardchar.size() > static_cast<size_t>(n)) {
    // Trim rows to match the required number of rows
    boardchar.resize(static_cast<size_t>(n));
}

// For each row, ensure the number of columns matches 'm'
for (size_t i = 0; i < boardchar.size(); ++i) {
    if (boardchar[i].size() < static_cast<size_t>(m)) {
        // Pad with spaces if columns are fewer than 'm'
        boardchar[i].resize(static_cast<size_t>(m), ' ');
    } else if (boardchar[i].size() > static_cast<size_t>(m)) {
        // Trim columns if there are more than 'm'
        boardchar[i].resize(static_cast<size_t>(m));
    }
}
// Check if trimming cut off '1' or '2'
bool fatalError = false;
count_1 = 0, count_2 = 0;
for (int i = 0; i < n; ++i) {
    for (int j = 0; j < m; ++j) {
        if (boardchar[i][j] == '1') count_1++;
        if (boardchar[i][j] == '2') count_2++;
    }
}

// If there's no '1' or '2' or they were trimmed off, it's a fatal error
if (count_1 == 0 || count_2 == 0) {
    errors.push_back("Fatal error: Missing '1' or '2'.");
    export_errors(std::string(argv[1]) + ".errors.txt", errors);
    return 1;
}

std::string input_filename = argv[1];

// Check if the filename ends with ".txt" and remove it
if (input_filename.size() >= 4 && input_filename.substr(input_filename.size() - 4) == ".txt") {
    input_filename = input_filename.substr(0, input_filename.size() - 4); // Remove ".txt"
}

// Export errors to the new file, appending "_errors.txt"
if (!errors.empty()) {
    export_errors(input_filename + "_errors.txt", errors); // Save errors to <input_file>_errors.txt
}
    file.close();

      // Create a 2D vector of cells to represent the game board
      vector<vector<cell>> board_cells(n, vector<cell>(m));
      for (int i = 0; i < n; ++i) {
          for (int j = 0; j < m; ++j) {
              board_cells[i][j].x = i; // Set the row index
              board_cells[i][j].y = j; // Set the column index
          }
      }
  
      // Initialize the game board
      game_board board(n, m, board_cells);
  
      // Loop through the boardchar to place tanks, walls, and mines
      for (int i = 0; i < n; ++i) {
          for (int j = 0; j < m; ++j) {
              char cell = boardchar[i][j];
              if (cell == '1') {
                  // Place Tank 1
                  tank* tank1 = new tank('1', 0, -1, &board.arr[i][j]);
                  board.add_tank(tank1);
              } else if (cell == '2') {
                  // Place Tank 2
                  tank* tank2 = new tank('2', 0, 1, &board.arr[i][j]);
                  board.add_tank(tank2);
              } else if (cell == '#') {
                  // Place Wall
                  wall* new_wall = new wall('w', &board.arr[i][j]);
              } else if (cell == '@') {
                  // Place Mine
                  mine* new_mine = new mine('m', &board.arr[i][j]);
              }
          }
      }
    // Create and place Tank 1
    // int a = 4, b = 2;
    // tank* tank1 = new tank('1', 0, 1, &board.arr[a][b]);
    // board.add_tank(tank1);

    // Create and place Tank 2
    // int c = 5, d = 8;
    // tank* tank2 = new tank('2', 0, -1, &board.arr[c][d]);
    // board.add_tank(tank2);

    // Add walls to the board
    // wall* wall1 = new wall('w', &board.arr[1][1]); // Wall at (1, 1)
    // wall* wall2 = new wall('w', &board.arr[2][1]); // Wall at (1, 1)
    // wall* wall3 = new wall('w', &board.arr[3][5]); // Wall at (3, 5)
    // wall* wall4 = new wall('w', &board.arr[4][5]); // Wall at (3, 5)

    // wall* wall5 = new wall('w', &board.arr[8][7]); // Wall at (3, 5)
    // wall* wall6 = new wall('w', &board.arr[8][6]); // Wall at (3, 5)
    // wall* wall7 = new wall('w', &board.arr[8][5]); // Wall at (3, 5)

    // Add mines to the board
    // mine* mine1 = new mine('m', &board.arr[0][4]); // Mine at (0, 4)
    // mine* mine2 = new mine('m', &board.arr[4][3]); // Mine at (4, 2)

    // Initialize algorithms
    algorithm* algo1 = new running_algorithm();
    algorithm* algo2 = new chasing_algorithm();

    // Initialize the game master
    game_master master(&board, algo1, algo2);

    // Start the game
    master.start_game();
    master.game();

    // Clean up dynamically allocated memory
    delete algo1;
    delete algo2;
    
    board.destroy_all_objects(); // Destroy all objects on the board

    return 0;
}

