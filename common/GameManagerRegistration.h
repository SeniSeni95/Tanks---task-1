#pragma once
#include <functional>
#include <memory>

// Forward declaration
class AbstractGameManager;

struct GameManagerRegistration {
    GameManagerRegistration(std::function<std::unique_ptr<AbstractGameManager>(bool)>);
};

#define REGISTER_GAME_MANAGER(class_name) \
    GameManagerRegistration register_me_##class_name \
        ( [] (bool verbose) { return std::make_unique<class_name>(verbose); } );
