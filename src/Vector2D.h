#pragma once
#ifndef VECTOR2D_H
#define VECTOR2D_H
struct Vector2D {
    int x, y;

    Vector2D operator+(const Vector2D& other) const;
    Vector2D operator*(double scalar) const;
    Vector2D operator-(const Vector2D& other) const;

    // Chebyshev (L∞) norm
    int chebyshevDistance(const Vector2D& other) const;
};
#endif