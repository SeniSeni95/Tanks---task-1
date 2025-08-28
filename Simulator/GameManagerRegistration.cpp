#include "GameManagerRegistration.h"

GameManagerRegistration::GameManagerRegistration(
    std::function<std::unique_ptr<AbstractGameManager>(bool)> f) {
    GameManagerRegistrar::get().addFactory(std::move(f));
}
