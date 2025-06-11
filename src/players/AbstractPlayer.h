#pragma once
#include "Player.h"
#include "TankAlgorithm.h"
#include "SatelliteView.h"
#include "SatelliteViewImpl.h"
#include "BattleItem.h"
#include "MyBattleInfo.h"
#include "Board.h"
#include "utils.h"
#include <vector>

using namespace std;

bool isAllyTank(char symbol, int player_index);

bool isEnemyTank(char symbol, int player_index);

char isTank(char symbol, int player_index);

class AbstractPlayer : public Player
{
private:
    int player_index; // Player index
    size_t width; // Width of the game board
    size_t height; // Height of the game board
    size_t max_steps; // Maximum steps allowed
    size_t num_shells; // Number of shells available

    bool boardInitialized = false; // Flag to check if the board is initialized
    game_board board;

public:
    AbstractPlayer(int player_index, size_t x, size_t y,
                     size_t max_steps, size_t num_shells);

    void updateTankWithBattleInfo(TankAlgorithm &tankAlg, SatelliteView &satellite_view) override;

    /**
     * Update the board by moving the tanks and shells and updating their directions.
     * Position updates are done on a closest-to basis.
     * Direction updates are done based on the direction between the previous position and the current one (rounded to 8 directions).
     */
    void updateBoard(const SatelliteViewImpl *view);

    /**
     * Find the closest tank to a given position with a specific symbol.
     */
    tank* findClosestTank(Vector2D target_pos, char symbol);

    /**
     * Find the closest shell to a given position, which matches the shells movement (direction and moves 2 steps at a time).
     */
    shell* findClosestShell(Vector2D target_pos);


    /**
     * Initialize the game board with initial tank data from the satellite view.
     */
    void initBoard(const SatelliteViewImpl *view);

    /**
     * Parse tanks from the initial satellite view.
     */
    vector<tuple<int, int, int, int, string>> initialParseSatView(const SatelliteViewImpl *view);

    virtual tuple<int, int, int, int, string> initTank(const SatelliteViewImpl *view, int x, int y);
};
