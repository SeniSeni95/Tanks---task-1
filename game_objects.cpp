#include <iostream>
#include <vector>
#include <string>
#include <math.h>
#include <algorithm>
#include "game_objects.h"
#include "board.h"
#include "utils.h"
using namespace std;
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Forward declarations
struct game_board;
struct cell;

struct game_object;
struct tank;
struct shell;
struct mine;
struct wall;

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

struct shell : public game_object {
    int directionx;
    int directiony;
    cell* curcell; // Pointer to the current cell
    string shell_symbol;

    // Updated constructor to store curcell as a pointer
    shell(cell* curcell, int directionx, int directiony) {
        this->curcell = curcell; // Store the dereferenced value
        this->x = curcell->get_X(); // Initialize x and y based on the cell's coordinates
        this->y = curcell->get_Y(); // Initialize x and y based on the cell's coordinates

        this->directionx=directionx;
        this->directiony=directiony;
        this->set_shell_symbol();
        curcell->add_Object(this); // Add the shell to the current cell
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
    void set_shell_symbol() {
        double degree = atan2(-directionx, directiony) * (180.0 / M_PI);
        if (degree < 0) {
            degree += 360;
        } else if (degree >= 360) {
            degree -= 360;
        }
        if (degree == 0) {
            shell_symbol = "→";
        } else if (degree == 90) {
            shell_symbol = "↑";
        } else if (degree == 180) {
            shell_symbol = "←";
        } else if (degree == 270) {
            shell_symbol = "↓";
        } else if (degree == 45) {
            shell_symbol = "↗";
        } else if (degree == 135) {
            shell_symbol = "↖";
        } else if (degree == 225) {
            shell_symbol = "↙";
        } else if (degree == 315) {
            shell_symbol = "↘";
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
        std::pair<int, int> new_direction = ::rotate_4(directionx, directiony, direction);
        directionx = new_direction.first;
        directiony = new_direction.second;
        set_cannon_symbol();
    }

    void rotate_8(string direction) {
        std::pair<int, int> new_direction = ::rotate_8(directionx, directiony, direction);
        directionx = new_direction.first;
        directiony = new_direction.second;
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
            shell* s = new shell(curcell, directionx, directiony);
            board->add_shell(s); // Add the shell to the game board
        }
    }

    bool handle_move(game_board* board, const string& move) {
        if (move == "fw") {
            // Perform wall collision check for the new position after moving forward
            if (wall_coll_check(&board->arr[(x + directionx + board->n) % board->n][(y + directiony + board->m) % board->m])) {
                cout << "Wall collision detected!" << endl; // Handle collision
                return false; // Return false, meaning the move is invalid
            } else {
                move_forward(*board); // Move the tank forward
                gear = "forward"; // Set gear to forward
                return true;
            }
        } else if (move == "r4l") {
            rotate_4("left");
            gear = "forward"; // Set gear to forward
            return true;
        } else if (move == "r8l") {
            rotate_8("left");
            gear = "forward"; // Set gear to forward
            return true;
        } else if (move == "r4r") {
            rotate_4("right");
            gear = "forward"; // Set gear to forward
            return true;
        } else if (move == "r8r") {
            rotate_8("right");
            gear = "forward"; // Set gear to forward
            return true;
        } else if (move == "shoot") {
            // Check if the tank's shoot timer is 0
            if (shot_timer == 0) {
                shoot(board); // Allow the tank to shoot
                gear = "forward"; // Set gear to forward
                return true;
            } else {
                cout << "Tank " << symbol << " isn't ready to shoot yet!" << endl;
                return false; // Return false, meaning the move is invalid
            }
        } else {
            cout << "Invalid move!" << endl;
            return false; // Invalid move
        }
    }

    bool wall_coll_check(cell* dest) {
        // Check if the destination cell is empty or contains a wall
        if (dest->has_Object()) {
            game_object* obj = dest->get_Object();
            if (obj->symbol == 'w') {
                return true; // Collision with a wall
            }
        }
        return false; // No collision
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