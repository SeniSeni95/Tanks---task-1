#pragma once
#include "GameManagerRegistrar.h"

struct GameManagerRegistration {
    GameManagerRegistration(std::function<std::unique_ptr<AbstractGameManager>()> f) {
        GameManagerRegistrar::get().addFactory(std::move(f));
    }
};
