#ifndef UTILS_H
#define UTILS_H

#include <cmath>
#include <algorithm>
#include <limits>

struct Vector2D {
    double x, y;

    Vector2D operator+(const Vector2D& other) const;
    Vector2D operator*(double scalar) const;
    Vector2D operator-(const Vector2D& other) const;

    // Chebyshev (L∞) norm
    double chebyshevDistance(const Vector2D& other) const;
};

// Finds the closest Chebyshev distance between a point and a parametric line
double chebyshevDistanceToLine(const Vector2D& linePoint, const Vector2D& lineDir, const Vector2D& point);

std::pair<int, int> rotate_4(int directionx, int directiony, std::string direction);
std::pair<int, int> rotate_8(int directionx, int directiony, std::string direction);

#endif // UTILS_H