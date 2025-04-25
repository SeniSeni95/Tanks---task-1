#include <iostream>
#include <vector>
#include <string>
#include <math.h>
#include <algorithm>
using namespace std;
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

struct tank;
struct shell;
struct mine;
struct wall;
struct game_object;
struct cell;
struct game_board;

struct game_object {
    int x;
    int y; 
    char symbol; 
    public:
    game_object(int x, int y, char symbol) {
        this->x = x;
        this->y = y;
        this->symbol = symbol;
    }
    game_object() {
        this->x = 0;
        this->y = 0;
        this->symbol = ' ';
    }
    char get_symbol() {
        return symbol;
    }
    void set_symbol(char s) {
        symbol = s;
    }
    void set_x(int x) {
        this->x = x;
    }   
    void set_y(int y) {
        this->y = y;
    }
    int get_x() {
        return x;
    }
    int get_y() {
        return y;
    }
    virtual void print(){}
};


struct cell {
    int x;
    int y;
    vector<game_object*> objects; // Vector to store multiple game_object pointers

public:
    cell(int x, int y) {
        this->x = x;
        this->y = y;
    }   
    cell(int x, int y, game_object* obj) {
        this->x = x;
        this->y = y;
        this->objects.push_back(obj); // Add the object to the vector
    }   
    cell() {
        this->x = 0;
        this->y = 0;
    } 

    void set_X(int x) {
        this->x = x;
    }
    void set_Y(int y) {
        this->y = y;
    }
    int get_X() {
        return x;
    }
    int get_Y() {
        return y;
    }
    bool has_Object() {
        return !objects.empty(); // Check if the vector is not empty
    }
    game_object* get_Object() {
        return objects[0]; // Return the vector of objects
    }
    void add_Object(game_object* obj) {
        this->objects.push_back(obj); // Add an object to the vector
    }
    void remove_Object(game_object* obj) {
        // Remove the specific object from the vector
        objects.erase(remove(objects.begin(), objects.end(), obj), objects.end());
    }
    void print() {
        if (!objects.empty()) {
            objects[0]->print(); // Call the print method of the first object
        } else {
            cout << "[  ]"; // Print an empty cell if no objects are present
        }
    }
};

struct game_board {
    int n; // number of rows
    int m; // number of columns
    vector<tank*> tanks;
    vector<shell*> shells;
    vector<vector<cell>> arr; // 2D array to represent the game board
    vector<cell*> collisions;

    game_board(int n, int m, vector<vector<cell>> arr) {
        this->n = n;
        this->m = m;
        this->arr = arr; // Initialize the game board with the provided 2D vector
    }
    

    void add_tank(tank* t) {
        tanks.push_back(t); // Add a tank to the tanks vector
    }

    void remove_tank(tank* t) {
        tanks.erase(remove(tanks.begin(), tanks.end(), t), tanks.end()); // Remove a tank
    }

    void add_shell(shell* s) {
        shells.push_back(s); // Add a shell to the shells vector
    }

    void remove_shell(shell* s) {
        shells.erase(remove(shells.begin(), shells.end(), s), shells.end()); // Remove a shell
    }

    void print_board() {
        // Function to print the game board
        for (int i = 0 ; i < n; i++) {  // Flip vertical
            for (int j = 0; j < m; ++j) {
                arr[i][j].print(); // Call the print method of each cell
            }
            cout << endl;
        }
    }
};




struct shell : public game_object {
    int directionx;
    int directiony;
    cell* curcell; // Pointer to the current cell
    string shell_symbol;

    // Updated constructor to store curcell as a pointer
    shell(cell* curcell) {
        this->curcell = curcell; // Store the dereferenced value
        this->x = curcell->get_X(); // Initialize x and y based on the cell's coordinates
        this->y = curcell->get_Y(); // Initialize x and y based on the cell's coordinates
    }
    void shell_move_forward(game_board& board) {
    // Remove the shell from the current cell
    curcell->remove_Object(this);

    // Update the shell's position with wrapping
    x = (x + directionx + board.n) % board.n; // wrap over rows
    y = (y + directiony + board.m) % board.m; // wrap over columns

    // Access the new cell from the game board
    curcell = &board.arr[x][y];

    // Add the shell to the new cell
    curcell->add_Object(this);

    // Register collision if there are multiple objects and no mine is present
    if (curcell->objects.size() > 1 &&
        find(board.collisions.begin(), board.collisions.end(), curcell) == board.collisions.end()) {
        
        // Check if the cell contains a mine
        bool has_mine = false;
        for (game_object* obj : curcell->objects) {
            if (obj->symbol == 'm') {
                has_mine = true;
                break;
            }
        }

        if (!has_mine) {
            board.collisions.push_back(curcell);
        }
    }
}
    void set_shell_symbol(shell* bullet) {
        double degree = atan2(-directionx, directiony) * (180.0 / M_PI);
        if (degree < 0) {
            degree += 360;
        } else if (degree >= 360) {
            degree -= 360;
        }
        if (degree == 0) {
            bullet->shell_symbol = "→";
        } else if (degree == 90) {
            bullet->shell_symbol = "↑";
        } else if (degree == 180) {
            bullet->shell_symbol = "←";
        } else if (degree == 270) {
            bullet->shell_symbol = "↓";
        } else if (degree == 45) {
            bullet->shell_symbol = "↗";
        } else if (degree == 135) {
            bullet->shell_symbol = "↖";
        } else if (degree == 225) {
            bullet->shell_symbol = "↙";
        } else if (degree == 315) {
            bullet->shell_symbol = "↘";
        }
    }

    void print() {
        cout << "[ " << shell_symbol << "]"; // Print the shell's symbol and cannon symbol
    }
};

struct tank : public game_object {
    int shells;
    int directionx;
    int directiony;
    int shot_timer;
    string cannon_symbol;
    string gear;
    cell* curcell; // Pointer to the current cell
    


    tank(char symbol, int directiony, int directionx, cell* curcell) {
        this->curcell = curcell;
        curcell->add_Object(this); // Add the tank to the current cell
        this->x =curcell->get_X(); // Initialize x and y based on the cell's coordinates
        this->y = curcell->get_Y(); // Initialize x and y based on the cell's coordinates 
        this->symbol = symbol;
        this->directionx = directiony;
        this->directiony = directionx;
        set_cannon_symbol(); // Set the cannon symbol based on the tank's direction
        this->shells=16;
        this->shot_timer=0;
        this->gear="forward";
        
    }

    void move_forward(game_board& board) {
        // Remove the tank from the current cell
        curcell->remove_Object(this);
        
        // Debug print to check the current position before moving
        // cout << "Current Position before move: (" << x << ", " << y << ")" << endl;
    
        // Update the tank's position with wrapping
        x = (x + directionx + board.n) % board.n; // wrap over rows (vertical)
        y = (y + directiony + board.m) % board.m; // wrap over columns (horizontal)
    
        // Debug print to check the new position after move
        // cout << "New Position after move: (" << x << ", " << y << ")" << endl;
    
        // Access the new cell (row-major order)
        curcell = &board.arr[x][y]; // Correct cell assignment: x = row, y = column
        
        // Add the tank to the new cell
        curcell->add_Object(this);
    
        // Check for collisions
        if (curcell->objects.size() > 1) {
            board.collisions.push_back(curcell); // Add the cell to the collisions vector
        }
    }
    
    string get_cannon_symbol() {
        return cannon_symbol;
    }
    void set_cannon_symbol() {
        double degree = atan2(-directionx, directiony) * (180.0 / M_PI);
        if (degree < 0) {
            degree += 360;
        } else if (degree >= 360) {
            degree -= 360;
        }
        if (degree==0) {
            cannon_symbol="→";
        } else if (degree==90) {
            cannon_symbol="↑";
        } else if (degree==180) {
            cannon_symbol="←";
        } else if (degree==270) {
            cannon_symbol="↓";
        } else if (degree==45) {
            cannon_symbol="↗";
        } else if (degree==135) {
            cannon_symbol="↖";
        } else if (degree==225) {
            cannon_symbol="↙";
        } else if (degree==315) {
            cannon_symbol="↘";
        }
    }
    void rotate_4(string direction) {
       // Calculate the angle in radians
    double degree = atan2(directiony, directionx) * (180.0 / M_PI); // Convert radians to degrees
    if (direction == "left") {
        degree += 90;
    } else if (direction == "right") {
        degree -= 90;
    }
    // Normalize the angle to the range [0, 360)
    if (degree < 0) {
        degree += 360;
    } else if (degree >= 360) {
        degree -= 360;
    }
    // Convert degrees back to radians for trigonometric functions
    double radian = degree * (M_PI / 180.0);
    // Update directionx and directiony using cos and sin
    directionx = round(cos(radian)); // Round to avoid floating-point precision issues
    directiony = round(sin(radian));
    set_cannon_symbol();
    }

    void rotate_8(string direction) {
        // Calculate the angle in radians
     double degree = atan2(directiony, directionx) * (180.0 / M_PI); // Convert radians to degrees
     if (direction == "left") {
         degree += 45;
     } else if (direction == "right") {
         degree -= 45;
     }
     // Normalize the angle to the range [0, 360)
     if (degree < 0) {
         degree += 360;
     } else if (degree >= 360) {
         degree -= 360;
     }
     // Convert degrees back to radians for trigonometric functions
     double radian = degree * (M_PI / 180.0);
     // Update directionx and directiony using cos and sin
     directionx = round(cos(radian)); // Round to avoid floating-point precision issues
     directiony = round(sin(radian));
     set_cannon_symbol();
     }
     void move_backwards(game_board& board) {
        // Remove the tank from the current cell
        curcell->remove_Object(this);
    
        // Update the tank's position with wrapping
        x = (x - directionx + board.n) % board.n; // Wrap over rows (vertical)
        y = (y - directiony + board.m) % board.m; // Wrap over columns (horizontal)
    
        // Access the new cell from the game board
        curcell = &board.arr[x][y];
    
        // Add the tank to the new cell
        curcell->add_Object(this);
    
        // Register collision if there are multiple objects and the cell isn't already in the list
        if (curcell->objects.size() > 1 && 
            find(board.collisions.begin(), board.collisions.end(), curcell) == board.collisions.end()) {
            board.collisions.push_back(curcell); // Add the cell to the collisions vector
        }
    }
     void shoot(game_board* board){
        if (shells>0) {
            shells--;
            shot_timer=4;
            shell* s = new shell(curcell);
            s->directionx=directionx;
            s->directiony=directiony;
            s->set_shell_symbol(s);
            curcell->add_Object(s); // Add the shell to the current cell
            board->add_shell(s); // Add the shell to the game board
        }
     }

     void print() {
        cout << "[" << symbol << cannon_symbol << "]"; // Print the tank's symbol and cannon symbol
     }
};

struct mine : public game_object {
    cell* curcell; // Pointer to the current cell

    mine(char symbol, cell* curcell){
        this->curcell = curcell; // Initialize curcell
        this->symbol=symbol;
        this->x=curcell->get_X(); // Initialize x and y based on the cell's coordinates
        this->y=curcell->get_Y(); // Initialize x and y based on the cell's coordinates
        curcell->add_Object(this); // Add the mine to the current cell
    }
    void print() {
        cout << "[ " << symbol << "]"; 
    }
};

struct wall : public game_object {
    int hp ;
    cell* curcell;
    wall(char symbol,cell* curcell) {
        this->curcell = curcell; // Initialize curcell
        this->symbol=symbol;
        this->x=curcell->get_X(); // Initialize x and y based on the cell's coordinates
        this->y=curcell->get_Y(); // Initialize x and y based on the cell's coordinates
        curcell->add_Object(this); // Add the mine to the current cell
        this->hp=2;
    }
    void print() {
        cout << "[ " << symbol << "]"; 
    }
};

struct game_master{
    game_board* board;
    bool game_over;
    bool out_of_shells;
    int time_out_steps;
    game_master(game_board* board) {
        this->board = board; // Initialize the game board
        this->game_over = false; // Set game_over to false initially
        this->time_out_steps=40;
        this->out_of_shells=false;
    }
    void start_game() {
        // Function to start the game
        cout << "Game started!" << endl;
        // Add game logic here
    }
    void game() {
        board->print_board(); // Print the initial game board
        while ((time_out_steps >= 0) && (game_over == false)) {
            // Check if all tanks are out of shells
            out_of_shells = true; // Assume all tanks are out of shells
            for (tank* t : board->tanks) {
                if (t->shells > 0) {
                    out_of_shells = false; // If any tank has shells, set to false
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
            process_shells();
            if (!board->collisions.empty()) {
                handle_cell_collisions(); // Handle cell collisions
            }
            process_shells();
            if (!board->collisions.empty()) {
                handle_cell_collisions(); // Handle cell collisions
            }
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
    void handle_cell_collisions() {
        for (cell* c : board->collisions) {
            if (c->objects.size() >= 2) {
                cout << "Collision detected in cell (" << c->x << ", " << c->y << ")!" << endl;
    
                game_object* first_obj = c->objects[0];
    
                if (mine* m = dynamic_cast<mine*>(first_obj)) {
                    cout << "Mine detected in collision!" << endl;
                    for (auto it = c->objects.begin(); it != c->objects.end();) {
                        game_object* obj = *it;
                        if (tank* t = dynamic_cast<tank*>(obj)) {
                            cout << "Destroying tank " << t->symbol << " due to mine!" << endl;
                            board->remove_tank(t);
                            c->remove_Object(t);
                            delete t;
                            it = c->objects.begin(); // Restart after modification
                        } else {
                            ++it;
                        }
                    }
                    c->remove_Object(m);
                    delete m;
                    game_over = true;
                }
    
                else if (shell* s = dynamic_cast<shell*>(first_obj)) {
                    cout << "Shell detected in collision!" << endl;
                    for (auto it = c->objects.begin(); it != c->objects.end();) {
                        game_object* obj = *it;
                        if (tank* t = dynamic_cast<tank*>(obj)) {
                            cout << "Destroying tank " << t->symbol << " due to shell!" << endl;
                            board->remove_tank(t);
                            c->remove_Object(t);
                            delete t;
                            it = c->objects.begin(); // Restart after modification
                        } else {
                            ++it;
                        }
                    }
                    c->remove_Object(s);
                    board->remove_shell(s);
                    delete s;
                    game_over = true;
                }
    
                else if (tank* t = dynamic_cast<tank*>(first_obj)) {
                    cout << "Tank detected in collision!" << endl;
                    for (auto it = c->objects.begin(); it != c->objects.end();) {
                        game_object* obj = *it;
                        if (dynamic_cast<shell*>(obj) || dynamic_cast<mine*>(obj)) {
                            ++it;
                        } else {
                            cout << "Destroying object with symbol: " << obj->symbol << endl;
                            if (tank* tt = dynamic_cast<tank*>(obj)) {
                                board->remove_tank(tt);
                            }
                            c->remove_Object(obj);
                            delete obj;
                            it = c->objects.begin(); // Restart after modification
                        }
                    }
                    game_over = true;
                }
            }
        }
    
        // Clear collisions list after all processing
        board->collisions.clear();
    }
    
    
    bool turn(char player) {
        cout << "Player " << player << "'s turn!" << endl;
        string move = ask_algorithm(player); // Get the move from the player
    
        for (tank* t : board->tanks) {
            if (t->symbol == player) {
                // Handle blank input
                if (move == "skip") {
                    cout << "Player " << player << " skipped their turn." << endl;
                    return true;
                }
    
                // If gear is "middle", ignore all moves except "forward"
                if (t->gear == "middle" && move != "fw") {
                    cout << "Invalid move! Only 'forward' is allowed when gear is 'middle'." << endl;
                    return false;
                }
    
                // Handle "forward" move
                if (move == "fw") {
                    // Perform wall collision check for the new position after moving
                    if (wall_coll_check(t, &board->arr[(t->x + t->directionx + board->n) % board->n][(t->y + t->directiony + board->m) % board->m])) {
                        cout << "Wall collision detected!" << endl; // Handle collision
                        return false; // Return false, meaning the move is invalid
                    } else {
                        t->move_forward(*board); // Move the tank forward
                    }
                    t->gear = "forward"; // Set gear to "forward"
                }
                // Handle "backward" move
                else if (move == "bw") {
                    // Perform wall collision check for the new position after moving backward with wrapping
                    if (wall_coll_check(t, &board->arr[(t->x - t->directionx + board->n) % board->n][(t->y - t->directiony + board->m) % board->m])) {
                        cout << "Wall collision detected!" << endl; // Handle collision
                        return false; // Return false, meaning the move is invalid
                    } else {
                        t->move_backwards(*board); // Move the tank backward
                    }
                }
                // Handle other moves
                else if (move == "r4l") {
                    t->rotate_4("left");
                    t->gear = "forward"; // Set gear to "forward"
                } else if (move == "r8l") {
                    t->rotate_8("left");
                    t->gear = "forward"; // Set gear to "forward"
                } else if (move == "r4r") {
                    t->rotate_4("right");
                    t->gear = "forward"; // Set gear to "forward"
                } else if (move == "r8r") {
                    t->rotate_8("right");
                    t->gear = "forward"; // Set gear to "forward"
                } else if (move == "shoot") {
                    // Check if the tank's shoot timer is 0
                    if (t->shot_timer == 0) {
                        t->shoot(board); // Allow the tank to shoot
                        t->gear = "forward"; // Set gear to "forward"
                    } else {
                        cout << "Tank " << t->symbol << " isn't ready to shoot yet!" << endl;
                        return false; // Return false, meaning the move is invalid
                    }
                } else {
                    return false;
                }
            }
        }
        return true; // Return true if the turn was successful
    }
    bool wall_coll_check(tank* t1, cell* dest) {
            // Check if the destination cell is empty or contains a wall
            if (dest->has_Object()) {
                game_object* obj = dest->get_Object();
                if (obj->symbol == 'w') {
                    return true; // Collision with a wall
                }
            }
            return false; // No collision
    }
    
    void process_shells() {
        // Process the shells on the board
        for (shell* s : board->shells) {
            s->shell_move_forward(*board); // Move the shell forward
    
            if (s->curcell->has_Object()) {
                game_object* obj = s->curcell->get_Object();
                
                // If the object is a wall, handle the collision with the wall
                if (obj->symbol == 'w') {
                    if (wall* w = dynamic_cast<wall*>(obj)) {
                        w->hp--;
                        if (w->hp <= 0) {
                            s->curcell->remove_Object(w); // Remove the wall from the cell
                            delete w; // Delete the wall object
                        }
                        s->curcell->remove_Object(s);
                        board->remove_shell(s); // Remove the shell from the board
                        delete s; // Delete the shell object
                    }
                }
    
                // No need to handle shell + mine here anymore
            }
        }
    }
    
    string ask_algorithm(char player) {
        // Function to ask the player for their move
        string move;
        cout << "Enter your move (forward/backward/rotate left/rotate right/shoot): ";
        cin >> move; // Get the move from the player
        return move; // Return the move
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
    int a = 3, b = 2;
    tank* tank1 = new tank('1', -1, 0, &board.arr[a][b]); // Tank 1 at (3, 2)
    board.add_tank(tank1);

    // Create and place Tank 2
    int c = 1, d = 4;
    tank* tank2 = new tank('2', -1, 0, &board.arr[c][d]); // Tank 2 at (1, 6)
    board.add_tank(tank2);

    // Add walls to the board
    wall* wall1 = new wall('w', &board.arr[1][1]); // Wall at (1, 1)
    wall* wall2 = new wall('w', &board.arr[3][5]); // Wall at (3, 5)

    // Add mines to the board
    mine* mine1 = new mine('m', &board.arr[0][4]); // Mine at (0, 4)
    mine* mine2 = new mine('m', &board.arr[4][2]); // Mine at (4, 2)

    // Initialize the game master
    game_master master(&board);

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

