#pragma once
#include "ActionRequest.h"
#include "TankAlgorithm.h"
#include "MyBattleInfo.h"
#include "GameManager.h"
#include "algorithms.h"

using namespace std;

class AbstractTankAlgorithm : public TankAlgorithm {
private:
    int playerIndex; // Player index
    int tankIndex; // Tank index
    int stepsSinceBoardUpdate = 0; // Steps since the last board update
protected:
    unique_ptr<algorithm> algo; // Pointer to the algorithm used by this tank

    unique_ptr<game_board> board; // Pointer to the game board
    shared_ptr<tank> selfTank = nullptr; // Pointer to the self tank
public:
    AbstractTankAlgorithm(int player_index, int tank_index);
    virtual ~AbstractTankAlgorithm() = default;
    ActionRequest getAction() override;
    virtual ActionRequest getActionInternal();

    void updateBattleInfo(BattleInfo& info) override;

    virtual unique_ptr<algorithm> createAlgorithm() = 0;
    virtual void initialize();
};
