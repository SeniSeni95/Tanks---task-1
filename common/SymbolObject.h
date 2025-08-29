#pragma once
#include "GameObject.h"


// Debug control - set to true to enable debugging, false to disable
inline constexpr bool DEBUG_SYMBOL = true;

class SymbolObject : public game_object {
public:
   SymbolObject(int x, int y, char s)
    : game_object(x, y, s) {
    if (DEBUG_SYMBOL)
        std::cout << "[DEBUG] SymbolObject ctor at (" << x << "," << y << ") with '" << s << "'\n";
}

    char get_symbol() const override {
    if (DEBUG_SYMBOL)
        std::cout << "[DEBUG] SymbolObject::get_symbol() returns '" << symbol << "'\n";
    return symbol;
}

    std::string to_string() override {
        return "[ " + std::string(1, symbol) + " ]";
    }
};
