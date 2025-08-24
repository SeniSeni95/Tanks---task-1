PlayerRegistration::PlayerRegistration(PlayerFactory factory) {
    auto& regsitrar = AlgorithmRegistrar::getAlgorithmRegistrar();
    regsitrar.addPlayerFactoryToLastEntry(std::move(factory));
}
