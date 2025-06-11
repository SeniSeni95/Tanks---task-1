#include "Vector2D.h"

class BattleItem {
    Vector2D position; // Position of the item
    Vector2D direction; // Direction of the item
    bool directionKnown; // Whether the direction is known

public:
    BattleItem(const Vector2D& position, const Vector2D& direction, bool directionKnown)
        : position(position), direction(direction), directionKnown(directionKnown) {}

    BattleItem(int x, int y, int dx, int dy, bool directionKnown)
        : position{x, y}, direction{dx, dy}, directionKnown(directionKnown) {}

    BattleItem(int x, int y) 
        : position{x, y}, direction{0, 0}, directionKnown(false) {}

    // Getters
    const Vector2D& getPosition() const { return position; }
    const Vector2D& getDirection() const { return direction; }
    bool isDirectionKnown() const { return directionKnown; }

    // Setters
    void setPosition(const Vector2D& newPosition) { position = newPosition; }
    void setPosition(int x, int y) { setPosition(Vector2D{x, y}); }

    void setDirection(const Vector2D& newDirection) { direction = newDirection; setDirectionKnown(true); }
    void setDirection(int dx, int dy) { setDirection(Vector2D{dx, dy}); }

    void setDirectionKnown(bool known) { directionKnown = known; }
};
