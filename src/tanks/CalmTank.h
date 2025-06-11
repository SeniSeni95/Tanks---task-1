#pragma once
#include "AbstractTankAlgorithm.h"

using namespace std;

class CalmTank : public AbstractTankAlgorithm {
    virtual unique_ptr<algorithm> createAlgorithm() override;
};
