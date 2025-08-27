#include "AlgorithmRegistrar.h"

// define the static instance
AlgorithmRegistrar AlgorithmRegistrar::registrar;

// static getter
AlgorithmRegistrar& AlgorithmRegistrar::getAlgorithmRegistrar() {
    return registrar;
}
