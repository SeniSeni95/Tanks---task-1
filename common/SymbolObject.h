#pragma once
#include "GameObject.h"
#include <string>

class SymbolObject : public game_object {
    char symbol;
public:
    // call base ctor (x, y, symbol)
    SymbolObject(int x, int y, char s)
        : game_object(x, y, s), symbol(s) {}

    // override get_symbol (non-const to match base)
    char get_symbol() override { return symbol; }

    // implement pure virtual
    std::string to_string() override {
        return std::string(1, symbol);
    }
};
