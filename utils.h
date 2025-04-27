#ifndef UTILS_H
#define UTILS_H

#include <cmath>
#include <algorithm>
#include <limits>
#include <string>

struct Vector2D {
    int x, y;

    Vector2D operator+(const Vector2D& other) const;
    Vector2D operator*(double scalar) const;
    Vector2D operator-(const Vector2D& other) const;

    // Chebyshev (L∞) norm
    int chebyshevDistance(const Vector2D& other) const;
};

// Finds the closest Chebyshev distance between a point and a parametric line
std::pair<int, int> chebyshevDistanceToLine(const Vector2D& linePoint, const Vector2D& lineDir, const Vector2D& point, int n, int m);

std::pair<int, int> rotate_4(int directionx, int directiony, std::string direction);
std::pair<int, int> rotate_8(int directionx, int directiony, std::string direction);

#endif // UTILS_H