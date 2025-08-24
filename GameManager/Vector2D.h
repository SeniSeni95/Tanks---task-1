#pragma once
#ifndef VECTOR2D_H
#define VECTOR2D_H

#include <cmath>
#include <algorithm>

class Vector2D {
public:
    int x, y;
    Vector2D(int x = 0, int y = 0) : x(x), y(y) {}
    Vector2D(size_t x = 0, size_t y = 0) : x(int(x)), y(int(y)) {}

    Vector2D operator+(const Vector2D& other) const;
    Vector2D operator*(double scalar) const;
    Vector2D operator-(const Vector2D& other) const;

    // Chebyshev (L∞) norm
    int chebyshevDistance(const Vector2D& other) const;
};
#endif
