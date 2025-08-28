#pragma once
#include "GameManagerRegistrar.h"

struct GameManagerRegistration {
    GameManagerRegistration(std::function<std::unique_ptr<AbstractGameManager>(bool)> f);
};
