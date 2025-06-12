#include "AbstractTankAlgorithm.h"

using namespace std;

AbstractTankAlgorithm::AbstractTankAlgorithm(int player_index, int tank_index)
    : playerIndex(player_index), tankIndex(tank_index)
{
}

void AbstractTankAlgorithm::initialize()
{
    algo = createAlgorithm();
    if (!algo)
    {
        throw std::runtime_error("Failed to create algorithm for AbstractTankAlgorithm");
    }
}

ActionRequest AbstractTankAlgorithm::getAction()
{
    ActionRequest action;
    if (!board)
    {
        action = ActionRequest::GetBattleInfo;
    }
    else
    {
        action = getActionInternal();
    }

    if (action == ActionRequest::GetBattleInfo)
    {
        stepsSinceBoardUpdate = 0;
    }
    else
    {
        stepsSinceBoardUpdate++;
        board->simulate_step(make_tuple(
            selfTank->get_x(),
            selfTank->get_y(),
            actionToString(action)));
    }

    return action;
}

ActionRequest AbstractTankAlgorithm::getActionInternal()
{
    // Use the algorithm to decide the action
    auto [action, score] = algo->decide_move(board.get(), selfTank, 2, stepsSinceBoardUpdate);
    return stringToAction(action);
}

void AbstractTankAlgorithm::updateBattleInfo(BattleInfo &info)
{
    MyBattleInfo *my_info = dynamic_cast<MyBattleInfo *>(&info);
    if (!my_info)
    {
        throw std::runtime_error("AbstractTankAlgorithm requires MyBattleInfo for battle info updates");
    }

    bool firstTime = !board;

    board = my_info->getBoard()->dummy_copy();

    if (firstTime)
    {
        for (auto &t : board->tanks)
        {
            if (t->player_number == playerIndex && t->tank_number == tankIndex)
            {
                selfTank = t;
                break;
            }
        }
    }

    if (!selfTank)
    {
        throw std::runtime_error("Self tank not found in the board");
    }
    
    my_info->setSelfTank(make_tuple(
            selfTank->get_x(),
            selfTank->get_y(),
            selfTank->directionx,
            selfTank->directiony,
            selfTank->gear));
}
