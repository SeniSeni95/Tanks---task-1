#ifndef UTILS_H
#define UTILS_H

#include <cmath>
#include <algorithm>
#include <limits>
#include <string>
#include "board.h"
#include "GameObject.h"

// Forward declarations
struct game_board;
struct cell;
struct game_object;
struct tank;
struct shell;
struct mine;
struct wall;

struct Vector2D {
    int x, y;

    Vector2D operator+(const Vector2D& other) const;
    Vector2D operator*(double scalar) const;
    Vector2D operator-(const Vector2D& other) const;

    // Chebyshev (L∞) norm
    int chebyshevDistance(const Vector2D& other) const;
};

// Finds the closest Chebyshev distance between a point and a parametric line
std::pair<int, int> chebyshevDistanceToLine(const Vector2D& linePoint, const Vector2D& lineDir, const Vector2D& point, game_board* board);

std::pair<int, int> rotate_4(int directionx, int directiony, std::string direction);
std::pair<int, int> rotate_8(int directionx, int directiony, std::string direction);

#endif // UTILS_H