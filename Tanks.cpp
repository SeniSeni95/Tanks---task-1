#include <iostream>
#include <vector>
#include <string>
#include <math.h>
#include <algorithm>
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
            bool t1 = turn('1'); // Call the turn function with player '1'
            while (t1 == false) {
                cout << "Invalid move! Please try again." << endl; // Handle invalid moves
                t1 = turn('1'); // Call the turn function with player '1'
            }
            bool t2 = turn('2'); // Call the turn function with player '2'
            while (t2 == false) {
                cout << "Invalid move! Please try again." << endl; // Handle invalid moves
                t2 = turn('2'); // Call the turn function with player '2'
            }
            game_over = board->do_step();
            board->print_board();
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
    
    
    bool turn(char player) {
        cout << "Player " << player << "'s turn!" << endl;
    
        for (tank* t : board->tanks) {
            if (t->symbol == player) {
                // Handle gear logic
                if (t->gear == "forward") {
                    string move = ask_algorithm(player); // Get the move from the player
    
                    if (move == "skip") {
                        cout << "Player " << player << " skipped their turn." << endl;
                        return true;
                    } else if (move == "bw") {
                        t->gear = "middle"; // Change gear to middle
                        cout << "Ordering backwards move!" << endl;
                        return true;
                    } else {
                        // Handle other moves (forward, rotate, shoot, etc.)
                        return t->handle_move(board, move);
                    }
                } else if (t->gear == "middle") {
                    string move = ask_algorithm(player); // Get the move from the player
    
                    if (move == "fw") {
                        t->gear = "forward"; // Change gear to forward
                        cout << "Tank is now ready to move forward!" << endl;
                        return true;
                    } else if (move == "skip") {
                        t->gear = "backwards move"; // Change gear to backwards move
                        cout << "Tank is now ready to move backwards!" << endl;
                        return true;
                    } else {
                        cout << "Invalid move! Only 'forward' or 'skip' is allowed in middle gear." << endl;
                        return false;
                    }
                } else if (t->gear == "backwards move") {
                    // Automatically move backwards without asking for input
                    t->move_backwards(*board);
                    t->gear = "backward"; // Change gear to backward
                    cout << "The tank moved backwards!" << endl;
                    return true;
                } else if (t->gear == "backward") {
                    string move = ask_algorithm(player); // Get the move from the player
    
                    if (move == "skip") {
                        cout << "Player " << player << " skipped their turn." << endl;
                        return true;
                    } else if (move == "bw") {
                        t->move_backwards(*board); // Move backwards
                        cout << "The tank moved backwards!" << endl;
                        return true;
                    } else {
                        // Handle other moves (forward, rotate, shoot, etc.)
                        t->gear = "forward"; // Change gear to forward
                        return t->handle_move(board, move);
                    }
                }
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
        if (player == '1') {
            return algo1->decide_move(board, board->tanks[0]).first; // Get the move from the first algorithm
        } else {
            return algo2->decide_move(board, board->tanks[1]).first; // Get the move from the second algorithm
        }
    }
};


int main() {
    int n = 5, m = 8; // Dimensions of the game board

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
    int a = 2, b = 1;
    tank* tank1 = new tank('1', 0, 1, &board.arr[a][b]);
    board.add_tank(tank1);

    // Create and place Tank 2
    int c = 2, d = 5;
    tank* tank2 = new tank('2', 0, -1, &board.arr[c][d]); 
    board.add_tank(tank2);

    // Add walls to the board
    wall* wall1 = new wall('w', &board.arr[1][1]); // Wall at (1, 1)
    wall* wall2 = new wall('w', &board.arr[3][5]); // Wall at (3, 5)

    // Add mines to the board
    mine* mine1 = new mine('m', &board.arr[0][4]); // Mine at (0, 4)
    mine* mine2 = new mine('m', &board.arr[4][2]); // Mine at (4, 2)

    // Initialize algorithms
    algorithm* algo1 = new shell_avoidance_algorithm();
    algorithm* algo2 = new shell_avoidance_algorithm();

    // Initialize the game master
    game_master master(&board, algo1, algo2);

    // Start the game
    master.start_game();
    master.game();

    // Clean up dynamically allocated memory
    delete tank1;
    delete tank2;
    delete wall1;
    delete wall2;
    delete mine1;
    delete mine2;

    return 0;
}
    // chcp 65001
    // git add .
    // git commit -m "your message"
    // git push

