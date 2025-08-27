#include "GameManagerRegistrar.h"

// define the static instance
GameManagerRegistrar GameManagerRegistrar::registrar;

GameManagerRegistrar& GameManagerRegistrar::get() {
    return registrar;
}
