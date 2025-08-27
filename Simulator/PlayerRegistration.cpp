#include "PlayerRegistration.h"   // <-- include your declaration
#include "AlgorithmRegistrar.h"

PlayerRegistration::PlayerRegistration(PlayerFactory factory) {
    auto& registrar = AlgorithmRegistrar::getAlgorithmRegistrar();
    registrar.addPlayerFactoryToLastEntry(std::move(factory));
}
