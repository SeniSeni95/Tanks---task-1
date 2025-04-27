#include <iostream>
#include <vector>
#include <string>
#include <math.h>
#include <algorithm>
#include <unistd.h>
#include "board.h"
#include "game_objects.h"
#include "algorithms.h"
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
    void game() {
        board->print_board(); // Print the initial game board
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

            bool t1 = turn('1', move1); // Call the turn function with player '1'
            bool t2 = turn('2', move2); // Call the turn function with player '2'
            game_over = board->handle_cell_collisions(); // Handle cell collisions

            if (game_over) {
                break; // Exit the loop if game is over
            }

            game_over = board->do_step(); // Perform half step again
            board->print_board();

            if (game_over) {
                break; // Exit the loop if game is over
            }

            sleep(1);
        }
        cout << "Game over!" << endl; // Print game over message
    
        // Print the board's tank vector
        cout << "Remaining tanks in board->tanks:" << endl;
        for (tank* t : board->tanks) {
            cout << " - Tank Symbol: " << t->symbol << ", Position: (" << t->x << ", " << t->y << ")" << endl;
        }
    
        if (board->tanks.size() == 1) {
            if (board->tanks[0]->symbol == '1') {
                cout << "Player 1 wins!" << endl; // Player 2 wins
            } else {
                cout << "Player 2 wins!" << endl; // Player 1 wins
            }
        } else if (board->tanks.size() == 0) {
            cout << "It's a tie!" << endl; // Player 1 wins
        } else if (board->tanks.size() == 2) {
            cout << "It's a tie!" << endl; // Game over
        }
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


int main() {
    int n = 10, m = 10; // Dimensions of the game board

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

    // Create and place Tank 1
    int a = 4, b = 2;
    tank* tank1 = new tank('1', 0, -1, &board.arr[a][b]);
    board.add_tank(tank1);

    // Create and place Tank 2
    int c = 5, d = 8;
    tank* tank2 = new tank('2', 0, 1, &board.arr[c][d]);
    board.add_tank(tank2);

    // Add walls to the board
    wall* wall1 = new wall('w', &board.arr[1][1]); // Wall at (1, 1)
    wall* wall2 = new wall('w', &board.arr[2][1]); // Wall at (1, 1)
    wall* wall3 = new wall('w', &board.arr[3][5]); // Wall at (3, 5)
    wall* wall4 = new wall('w', &board.arr[4][5]); // Wall at (3, 5)

    wall* wall5 = new wall('w', &board.arr[8][7]); // Wall at (3, 5)
    wall* wall6 = new wall('w', &board.arr[8][6]); // Wall at (3, 5)
    wall* wall7 = new wall('w', &board.arr[8][5]); // Wall at (3, 5)

    // Add mines to the board
    mine* mine1 = new mine('m', &board.arr[0][4]); // Mine at (0, 4)
    mine* mine2 = new mine('m', &board.arr[4][3]); // Mine at (4, 2)

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

