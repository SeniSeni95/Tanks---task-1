#pragma once
#include <vector>
#include <memory>
#include <functional>
#include "../common/AbstractGameManager.h"

class GameManagerRegistrar {
    std::vector<std::function<std::unique_ptr<AbstractGameManager>()>> factories;
    static GameManagerRegistrar registrar;

public:
    static GameManagerRegistrar& get();

    void addFactory(std::function<std::unique_ptr<AbstractGameManager>()> f) {
        factories.push_back(std::move(f));
    }

    auto begin() const { return factories.begin(); }
    auto end()   const { return factories.end(); }

    bool empty() const { return factories.empty(); }
    size_t count() const { return factories.size(); }

    void clear() { factories.clear(); }

    // return the most recently registered GM factory
    std::function<std::unique_ptr<AbstractGameManager>()> last() const {
        return factories.back();
    }
};
