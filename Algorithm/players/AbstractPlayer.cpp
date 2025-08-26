#include "AbstractPlayer.h"
#include <iostream>  // Added for debugging output

using namespace std;

// Debug control - set to true to enable debugging, false to disable
static const bool DEBUG_ENABLED = false;

bool isAllyTank(char symbol, int player_index)
{
    bool result = symbol == '%' || symbol == '0' + player_index;
    if (DEBUG_ENABLED) {
        cout << "[DEBUG] isAllyTank: symbol='" << symbol << "', player_index=" << player_index << ", result=" << result << endl;
    }
    return result;
}

bool isEnemyTank(char symbol, int player_index)
{
    int index = symbol - '0';
    bool result = 0 < index && index < 10 && index != player_index;
    if (DEBUG_ENABLED) {
        cout << "[DEBUG] isEnemyTank: symbol='" << symbol << "', player_index=" << player_index << ", index=" << index << ", result=" << result << endl;
    }
    return result;
}

char isTank(char symbol, int player_index)
{
    if (DEBUG_ENABLED) {
        cout << "[DEBUG] isTank: checking symbol='" << symbol 
             << "', player_index=" << player_index << endl;
    }

    if (symbol == '%') {
        char result = '0' + player_index;
        if (DEBUG_ENABLED) {
            cout << "[DEBUG] isTank: found shared ally '%' → returning '" 
                 << result << "'" << endl;
        }
        return result;
    }

    int index = symbol - '0';
    if (0 < index && index < 10) {
        if (index == player_index) {
            if (DEBUG_ENABLED) {
                cout << "[DEBUG] isTank: found ALLY tank '" << symbol 
                     << "' for player " << player_index << endl;
            }
            return '0' + player_index;  // Ally tank
        } else {
            if (DEBUG_ENABLED) {
                cout << "[DEBUG] isTank: found ENEMY tank '" << symbol 
                     << "' (belongs to player " << index << ")" << endl;
            }
            return symbol;  // Enemy tank
        }
    }

    if (DEBUG_ENABLED) {
        cout << "[DEBUG] isTank: not a tank, returning 0" << endl;
    }
    return 0; // Not a tank
}


AbstractPlayer::AbstractPlayer(int player_index, size_t x, size_t y,
                               size_t max_steps, size_t num_shells)
    : player_index(player_index),
      width(x),
      height(y),
      max_steps(max_steps),
      num_shells(num_shells) {
    if (DEBUG_ENABLED) {
        cout << "[DEBUG] AbstractPlayer constructor: player_index=" << player_index 
             << ", width=" << width << ", height=" << height 
             << ", max_steps=" << max_steps << ", num_shells=" << num_shells << endl;
    }
}

void AbstractPlayer::updateTankWithBattleInfo(TankAlgorithm &tankAlg, SatelliteView &satellite_view)
{
    // std::cout << "[DEBUG] updateTankWithBattleInfo: starting update for player "
            //   << player_index << "\n";

    if (!boardInitialized) {
        // std::cout << "[DEBUG] updateTankWithBattleInfo: board not initialized, calling initBoard\n";
        initBoard(satellite_view);
        boardInitialized = true;
    } else {
        // std::cout << "[DEBUG] updateTankWithBattleInfo: board already initialized, calling updateBoard\n";
        updateBoard(satellite_view);
    }

    // Just confirm that our own tank still exists in the view
    bool foundSelf = false;
    for (size_t i = 0; i < width; ++i) {
        for (size_t j = 0; j < height; ++j) {
            char sym = satellite_view.getObjectAt(i, j);
            if (isTank(sym, player_index)) {
                foundSelf = true;
                break;
            }
        }
        if (foundSelf) break;
    }

    if (!foundSelf) {
        throw std::runtime_error("Self tank not found in satellite view for player " +
                                 std::to_string(player_index));
    }

    // std::cout << "[DEBUG] updateTankWithBattleInfo: completed update for player "
            //   << player_index << "\n";
}




/**
 * Update the board by moving the tanks and shells and updating their directions.
 * Position updates are done on a closest-to basis.
 * Direction updates are done based on the direction between the previous position and the current one (rounded to 8 directions).
 */
void AbstractPlayer::updateBoard(SatelliteView &view)
{
    if (DEBUG_ENABLED) {
        cout << "[DEBUG] updateBoard: starting board update for player " << player_index << endl;
    }
    
    // Update tanks and shells based on the satellite view
    vector<tuple<int, int, int, int, string>> tank_data;
    vector<tuple<int, int, int, int>> shell_data;
    
    int tanks_found = 0;
    int shells_found = 0;
    
    for (size_t x = 0; x < width; ++x)
    {
        for (size_t y = 0; y < height; ++y)
        {
            Vector2D target_pos = {x, y};

            char symbol = view.getObjectAt(x, y);
            char real_symbol = isTank(symbol, player_index);
            if (real_symbol)
            {
                tanks_found++;
                if (DEBUG_ENABLED) {
                    cout << "[DEBUG] updateBoard: found tank '" << real_symbol << "' at (" << x << ", " << y << ")" << endl;
                }
                
                tank *closest_tank = findClosestTank(target_pos, real_symbol);
                if (closest_tank)
                {
                    Vector2D tank_pos = {closest_tank->get_x(), closest_tank->get_y()};
                    Vector2D direction = target_pos - tank_pos;
                    if (DEBUG_ENABLED) {
                        cout << "[DEBUG] updateBoard: closest tank at (" << tank_pos.x << ", " << tank_pos.y << "), direction (" << direction.x << ", " << direction.y << ")" << endl;
                    }
                    
                    if (direction.x == 0 && direction.y == 0)
                    {
                        if (DEBUG_ENABLED) {
                            cout << "[DEBUG] updateBoard: tank hasn't moved, keeping current direction" << endl;
                        }
                        tank_data.push_back(make_tuple(
                            closest_tank->get_x(),
                            closest_tank->get_y(),
                            closest_tank->directionx,
                            closest_tank->directiony,
                            closest_tank->gear));
                    } else {
                        int direction_x = 0, direction_y = 0;
                        if (direction.x > 0)
                            direction_x = 1;
                        else if (direction.x < 0)
                            direction_x = -1;
                        if (direction.y > 0)
                            direction_y = 1;
                        else if (direction.y < 0)
                            direction_y = -1;
                        string gear = "forward"; // Default gear, we don't predict gears.

                        if (DEBUG_ENABLED) {
                            cout << "[DEBUG] updateBoard: tank moved, new direction (" << direction_x << ", " << direction_y << ")" << endl;
                        }
                        tank_data.push_back(make_tuple(
                            x, y, direction_x, direction_y, gear));
                    }
                }
                else
                {
                    if (DEBUG_ENABLED) {
                        cout << "[DEBUG] updateBoard: no closest tank found for symbol '" << real_symbol << "'" << endl;
                    }
                }
            }
            else if (symbol == '*')
            {
                shells_found++;
                if (DEBUG_ENABLED) {
                    cout << "[DEBUG] updateBoard: found shell at (" << x << ", " << y << ")" << endl;
                }
                
                shell *closest_shell = findClosestShell(target_pos);
                if (closest_shell)
                {
                    int direction_x = closest_shell->directionx;
                    int direction_y = closest_shell->directiony;
                    if (DEBUG_ENABLED) {
                        cout << "[DEBUG] updateBoard: shell direction (" << direction_x << ", " << direction_y << ")" << endl;
                    }
                    shell_data.push_back(make_tuple(x, y, direction_x, direction_y));
                }
                else
                {
                    if (DEBUG_ENABLED) {
                        cout << "[DEBUG] updateBoard: no closest shell found" << endl;
                    }
                }
            }
        }
    }

    if (DEBUG_ENABLED) {
        cout << "[DEBUG] updateBoard: found " << tanks_found << " tanks and " << shells_found << " shells" << endl;
        cout << "[DEBUG] updateBoard: generating new board with " << tank_data.size() << " tank entries and " << shell_data.size() << " shell entries" << endl;
    }
    
    // Update the board with the new tank and shell data
    board = game_board::generate_board(view, width, height, shell_data, tank_data);
    
    if (DEBUG_ENABLED) {
        cout << "[DEBUG] updateBoard: board update completed for player " << player_index << endl;
    }
}

/**
 * Find the closest tank to a given position with a specific symbol.
 */
tank *AbstractPlayer::findClosestTank(Vector2D target_pos, char symbol)
{
    if (DEBUG_ENABLED) {
        cout << "[DEBUG] findClosestTank: searching for tank '" << symbol << "' closest to (" << target_pos.x << ", " << target_pos.y << ")" << endl;
    }
    
    tank *closest_tank = nullptr;
    double min_distance = std::numeric_limits<double>::max();
    int tanks_checked = 0;

    for (const auto &t : board->tanks)
    {
        tanks_checked++;
        if (t->get_symbol() == symbol)
        {
            Vector2D tank_pos = {t->get_x(), t->get_y()};
            double distance = tank_pos.chebyshevDistance(target_pos);
            if (DEBUG_ENABLED) {
                cout << "[DEBUG] findClosestTank: tank '" << symbol << "' at (" << tank_pos.x << ", " << tank_pos.y << "), distance=" << distance << endl;
            }
            
            if (distance < min_distance)
            {
                min_distance = distance;
                closest_tank = t.get();
                if (DEBUG_ENABLED) {
                    cout << "[DEBUG] findClosestTank: new closest tank found, distance=" << min_distance << endl;
                }
            }
        }
    }

    if (DEBUG_ENABLED) {
        cout << "[DEBUG] findClosestTank: checked " << tanks_checked << " tanks, ";
        if (closest_tank)
        {
            cout << "found closest at (" << closest_tank->get_x() << ", " << closest_tank->get_y() << "), distance=" << min_distance << endl;
        }
        else
        {
            cout << "no tank found with symbol '" << symbol << "'" << endl;
        }
    }

    return closest_tank;
}

/**
 * Find the closest shell to a given position, which matches the shells movement (direction and moves 2 steps at a time).
 */
shell *AbstractPlayer::findClosestShell(Vector2D target_pos)
{
    if (DEBUG_ENABLED) {
        cout << "[DEBUG] findClosestShell: searching for shell closest to (" << target_pos.x << ", " << target_pos.y << ")" << endl;
    }
    
    shell *closest_shell = nullptr;
    double min_distance = std::numeric_limits<double>::max();
    int shells_checked = 0;

    for (const auto &s : board->shells)
    {
        shells_checked++;
        int x = s->get_x();
        int y = s->get_y();
        Vector2D shell_pos = {x, y};
        Vector2D direction_to_shell = target_pos - shell_pos;
        // Compare with shells's actual direction
        int direction_x = s->directionx;
        int direction_y = s->directiony;
        
        if (DEBUG_ENABLED) {
            cout << "[DEBUG] findClosestShell: checking shell at (" << x << ", " << y << "), direction (" << direction_x << ", " << direction_y << ")" << endl;
        }
        
        if (direction_x == 0 && direction_to_shell.x != 0)
        {
            if (DEBUG_ENABLED) {
                cout << "[DEBUG] findClosestShell: shell direction mismatch (x=0 but target x != 0)" << endl;
            }
            continue;
        }
        if (direction_x < 0 && direction_to_shell.x >= 0)
        {
            if (DEBUG_ENABLED) {
                cout << "[DEBUG] findClosestShell: shell direction mismatch (x<0 but target x >= 0)" << endl;
            }
            continue;
        }
        if (direction_x > 0 && direction_to_shell.x <= 0)
        {
            if (DEBUG_ENABLED) {
                cout << "[DEBUG] findClosestShell: shell direction mismatch (x>0 but target x <= 0)" << endl;
            }
            continue;
        }
        if (direction_y == 0 && direction_to_shell.y != 0)
        {
            if (DEBUG_ENABLED) {
                cout << "[DEBUG] findClosestShell: shell direction mismatch (y=0 but target y != 0)" << endl;
            }
            continue;
        }
        if (direction_y < 0 && direction_to_shell.y >= 0)
        {
            if (DEBUG_ENABLED) {
                cout << "[DEBUG] findClosestShell: shell direction mismatch (y<0 but target y >= 0)" << endl;
            }
            continue;
        }
        if (direction_y > 0 && direction_to_shell.y <= 0)
        {
            if (DEBUG_ENABLED) {
                cout << "[DEBUG] findClosestShell: shell direction mismatch (y>0 but target y <= 0)" << endl;
            }
            continue;
        }

        if (DEBUG_ENABLED) {
            cout << "[DEBUG] findClosestShell: shell direction matches, simulating movement" << endl;
        }
        
        // Move 2 steps in the shell's direction until we reach the target position (or miss it - not this shell)
        int distance = 0;
        while (true)
        {
            shell_pos.x += direction_x;
            shell_pos.y += direction_y;
            shell_pos.x = (shell_pos.x + width) % width;   // Wrap around the x-coordinate
            shell_pos.y = (shell_pos.y + height) % height; // Wrap around the y-coordinate

            distance++;

            if (distance % 2 == 0)
            {
                // Target reached, update closest shell
                if (shell_pos.x == target_pos.x && shell_pos.y == target_pos.y)
                {
                    if (DEBUG_ENABLED) {
                        cout << "[DEBUG] findClosestShell: shell reaches target after " << distance << " steps" << endl;
                    }
                    if (distance < min_distance)
                    {
                        min_distance = distance;
                        closest_shell = s.get();
                        if (DEBUG_ENABLED) {
                            cout << "[DEBUG] findClosestShell: new closest shell found, distance=" << min_distance << endl;
                        }
                    }
                    break; // Stop checking this shell
                }
            }

            // If we got back to the original position, stop checking this shell
            if (shell_pos.x == x && shell_pos.y == y)
            {
                if (DEBUG_ENABLED) {
                    cout << "[DEBUG] findClosestShell: shell returned to original position, stopping simulation" << endl;
                }
                break; // Stop checking this shell
            }
            
            if (distance > width * height)  // Safety check to prevent infinite loops
            {
                if (DEBUG_ENABLED) {
                    cout << "[DEBUG] findClosestShell: safety break - too many steps" << endl;
                }
                break;
            }
        }
    }

    if (DEBUG_ENABLED) {
        cout << "[DEBUG] findClosestShell: checked " << shells_checked << " shells, ";
        if (closest_shell)
        {
            cout << "found closest at (" << closest_shell->get_x() << ", " << closest_shell->get_y() << "), distance=" << min_distance << endl;
        }
        else
        {
            cout << "no matching shell found" << endl;
        }
    }

    return closest_shell;
}

/**
 * Initialize the game board with initial tank data from the satellite view.
 */
void AbstractPlayer::initBoard(SatelliteView &view)
{
    if (DEBUG_ENABLED) {
        cout << "[DEBUG] initBoard: initializing board for player " << player_index << endl;
    }
    
    // Get initial tank data from the satellite view
    vector<tuple<int, int, int, int, string>> tank_data = initialParseSatView(view);
    
    if (DEBUG_ENABLED) {
        cout << "[DEBUG] initBoard: found " << tank_data.size() << " tanks during initial parse" << endl;
    }

    board = game_board::generate_board(
        view,
        width,
        height,
        std::vector<std::tuple<int, int, int, int>>(), // No shells initially
        tank_data);

    boardInitialized = true;
    if (DEBUG_ENABLED) {
        cout << "[DEBUG] initBoard: board initialization completed for player " << player_index << endl;
    }
}

/**
 * Parse tanks from the initial satellite view.
 */
vector<tuple<int, int, int, int, string>> AbstractPlayer::initialParseSatView(SatelliteView &view)
{
    if (DEBUG_ENABLED) {
        cout << "[DEBUG] initialParseSatView: parsing initial satellite view for player " << player_index << endl;
    }
    
    vector<tuple<int, int, int, int, string>> tank_data;
    int tanks_found = 0;
    
    // Iterate through the view to find all the items
    for (size_t x = 0; x < width; ++x)
    {
        for (size_t y = 0; y < height; ++y)
        {
            char symbol = view.getObjectAt(x, y);
            if (isTank(symbol, player_index))
            {
                tanks_found++;
                if (DEBUG_ENABLED) {
                    cout << "[DEBUG] initialParseSatView: found tank '" << symbol << "' at (" << x << ", " << y << ")" << endl;
                }
                tank_data.push_back(initTank(view, x, y));
            }
        }
    }

    if (DEBUG_ENABLED) {
        cout << "[DEBUG] initialParseSatView: completed parsing, found " << tanks_found << " tanks" << endl;
    }
    return tank_data;
}

tuple<int, int, int, int, string> AbstractPlayer::initTank(SatelliteView &view, int x, int y)
{
    char symbol = view.getObjectAt(x, y);
    if (DEBUG_ENABLED) {
        cout << "[DEBUG] initTank: initializing tank '" << symbol << "' at (" << x << ", " << y << ")" << endl;
    }

    int tank_player;
    if (isAllyTank(symbol, player_index))
    {
        tank_player = player_index; // Ally tank
        if (DEBUG_ENABLED) {
            cout << "[DEBUG] initTank: ally tank detected, player=" << tank_player << endl;
        }
    }
    else
    {
        tank_player = symbol - '0'; // Enemy tank
        if (DEBUG_ENABLED) {
            cout << "[DEBUG] initTank: enemy tank detected, player=" << tank_player << endl;
        }
    }

    if (tank_player == 1)
    {
        if (DEBUG_ENABLED) {
            cout << "[DEBUG] initTank: tank faces left (-1, 0)" << endl;
        }
        return make_tuple(x, y, -1, 0, "forward"); // Tank 1 faces left
    }
    else
    {
        if (DEBUG_ENABLED) {
            cout << "[DEBUG] initTank: tank faces right (1, 0)" << endl;
        }
        return make_tuple(x, y, 1, 0, "forward"); // Other tanks face right
    }
}
