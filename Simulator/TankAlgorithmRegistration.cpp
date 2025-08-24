TankAlgorithmRegistration::TankAlgorithmRegistration(MyTankAlgorithmFactory factory) {
    auto& regsitrar = AlgorithmRegistrar::getAlgorithmRegistrar();
    regsitrar.addTankAlgorithmFactoryToLastEntry(std::move(factory));
}
