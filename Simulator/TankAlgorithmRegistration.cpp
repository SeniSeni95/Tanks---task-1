#include "TankAlgorithmRegistration.h"   // <-- include your declaration
#include "AlgorithmRegistrar.h"

TankAlgorithmRegistration::TankAlgorithmRegistration(TankAlgorithmFactory factory) {
    auto& registrar = AlgorithmRegistrar::getAlgorithmRegistrar();
    registrar.addTankAlgorithmFactoryToLastEntry(std::move(factory));
}
