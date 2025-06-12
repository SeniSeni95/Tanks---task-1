#include "AbstractPlayer.h"

using namespace std;

bool isAllyTank(char symbol, int player_index)
{
    return symbol == '%' || symbol == '0' + player_index;
}

bool isEnemyTank(char symbol, int player_index)
{
    int index = symbol - '0';
    return 0 < index && index < 10 && index != player_index;
}

char isTank(char symbol, int player_index)
{
    if (symbol == '%')
    {
        return '0' + player_index;
    }

    int index = symbol - '0';
    if (0 < index && index < 10)
    {
        return symbol; // Return the symbol itself for enemy tanks
    }

    return 0; // Not a tank
}

AbstractPlayer::AbstractPlayer(int player_index, size_t x, size_t y,
                               size_t max_steps, size_t num_shells)
    : Player(player_index, x, y, max_steps, num_shells),
      player_index(player_index), width(x), height(y),
      max_steps(max_steps), num_shells(num_shells) {
      };

void AbstractPlayer::updateTankWithBattleInfo(TankAlgorithm &tankAlg, SatelliteView &satellite_view)
{
    if (!boardInitialized)
    {
        initBoard(satellite_view);
    }
    else
    {
        updateBoard(satellite_view);
    }

    // Clone board
    unique_ptr<game_board> board_copy = board->dummy_copy();
    // Create a BattleInfo object with the cloned board
    MyBattleInfo battle_info(std::move(board_copy));
    // Update the tank's algorithm with the battle info
    tankAlg.updateBattleInfo(battle_info);

    // The tank alg will give us some extra info it knows about its own tank
    tuple<int, int, int, int, string> self_tank = battle_info.getSelfTank();

    int x = get<0>(self_tank);
    int y = get<1>(self_tank);

    game_object *obj = board->get_cell(x, y).get_Object();
    if (!obj)
    {
        throw std::runtime_error("No object at self tank's position");
    }

    // Update the tank's position and direction
    int direction_x = get<2>(self_tank);
    int direction_y = get<3>(self_tank);
    string gear = get<4>(self_tank);
    tank *t = dynamic_cast<tank *>(obj);
    if (!t)
    {
        throw std::runtime_error("Object at self tank's position is not a tank");
    }
    t->set_x(x);
    t->set_y(y);
    t->directionx = direction_x;
    t->directiony = direction_y;
    t->gear = gear;
}

/**
 * Update the board by moving the tanks and shells and updating their directions.
 * Position updates are done on a closest-to basis.
 * Direction updates are done based on the direction between the previous position and the current one (rounded to 8 directions).
 */
void AbstractPlayer::updateBoard(SatelliteView &view)
{
    // Update tanks and shells based on the satellite view
    vector<tuple<int, int, int, int, string>> tank_data;
    vector<tuple<int, int, int, int>> shell_data;
    for (size_t x = 0; x < width; ++x)
    {
        for (size_t y = 0; y < height; ++y)
        {
            Vector2D target_pos = {x, y};

            char symbol = view.getObjectAt(x, y);
            char real_symbol = isTank(symbol, player_index);
            if (real_symbol)
            {
                tank *closest_tank = findClosestTank(target_pos, real_symbol);
                if (closest_tank)
                {
                    Vector2D tank_pos = {closest_tank->get_x(), closest_tank->get_y()};
                    Vector2D direction = target_pos - tank_pos;
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

                    tank_data.push_back(make_tuple(
                        x, y, direction_x, direction_y, gear));
                }
            }
            else if (symbol == '*')
            {
                shell *closest_shell = findClosestShell(target_pos);
                if (closest_shell)
                {
                    int direction_x = closest_shell->directionx;
                    int direction_y = closest_shell->directiony;
                    shell_data.push_back(make_tuple(x, y, direction_x, direction_y));
                }
            }
        }
    }

    // Update the board with the new tank and shell data
    board = game_board::generate_board(view, width, height, shell_data, tank_data);
}

/**
 * Find the closest tank to a given position with a specific symbol.
 */
tank *AbstractPlayer::findClosestTank(Vector2D target_pos, char symbol)
{
    tank *closest_tank = nullptr;
    double min_distance = std::numeric_limits<double>::max();

    for (const auto &t : board->tanks)
    {
        if (t->get_symbol() == symbol)
        {
            Vector2D tank_pos = {t->get_x(), t->get_y()};
            double distance = tank_pos.chebyshevDistance(target_pos);
            if (distance < min_distance)
            {
                min_distance = distance;
                closest_tank = t.get();
            }
        }
    }

    return closest_tank;
}

/**
 * Find the closest shell to a given position, which matches the shells movement (direction and moves 2 steps at a time).
 */
shell *AbstractPlayer::findClosestShell(Vector2D target_pos)
{
    shell *closest_shell = nullptr;
    double min_distance = std::numeric_limits<double>::max();

    for (const auto &s : board->shells)
    {
        int x = s->get_x();
        int y = s->get_y();
        Vector2D shell_pos = {x, y};
        Vector2D direction_to_shell = target_pos - shell_pos;
        // Compare with shells's actual direction
        int direction_x = s->directionx;
        int direction_y = s->directiony;
        if (direction_x == 0 && direction_to_shell.x != 0)
            continue;
        if (direction_x < 0 && direction_to_shell.x >= 0)
            continue;
        if (direction_x > 0 && direction_to_shell.x <= 0)
            continue;
        if (direction_y == 0 && direction_to_shell.y != 0)
            continue;
        if (direction_y < 0 && direction_to_shell.y >= 0)
            continue;
        if (direction_y > 0 && direction_to_shell.y <= 0)
            continue;

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
                    if (distance < min_distance)
                    {
                        min_distance = distance;
                        closest_shell = s.get();
                    }
                    break; // Stop checking this shell
                }
            }

            // If we got back to the original position, stop checking this shell
            if (shell_pos.x == x && shell_pos.y == y)
            {
                break; // Stop checking this shell
            }
        }
    }

    return closest_shell;
}

/**
 * Initialize the game board with initial tank data from the satellite view.
 */
void AbstractPlayer::initBoard(SatelliteView &view)
{
    // Get initial tank data from the satellite view
    vector<tuple<int, int, int, int, string>> tank_data = initialParseSatView(view);

    board = game_board::generate_board(
        view,
        width,
        height,
        std::vector<std::tuple<int, int, int, int>>(), // No shells initially
        tank_data);

    boardInitialized = true;
}

/**
 * Parse tanks from the initial satellite view.
 */
vector<tuple<int, int, int, int, string>> AbstractPlayer::initialParseSatView(SatelliteView &view)
{
    vector<tuple<int, int, int, int, string>> tank_data;
    // Iterate through the view to find all the items
    for (size_t x = 0; x < width; ++x)
    {
        for (size_t y = 0; y < height; ++y)
        {
            char symbol = view.getObjectAt(x, y);
            if (isTank(symbol, player_index))
            {
                tank_data.push_back(initTank(view, x, y));
            }
        }
    }

    return tank_data;
}

tuple<int, int, int, int, string> AbstractPlayer::initTank(SatelliteView &view, int x, int y)
{
    char symbol = view.getObjectAt(x, y);

    int tank_player;
    if (isAllyTank(symbol, player_index))
    {
        tank_player = player_index; // Ally tank
    }
    else
    {
        tank_player = symbol - '0'; // Enemy tank
    }

    if (tank_player == 1)
    {
        return make_tuple(x, y, -1, 0, "forward"); // Tank 1 faces left
    }
    else
    {
        return make_tuple(x, y, 1, 0, "forward"); // Other tanks face right
    }
}