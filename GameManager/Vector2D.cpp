#include "Vector2D.h"

Vector2D Vector2D::operator+(const Vector2D& other) const {
    return {x + other.x, y + other.y};
}

Vector2D Vector2D::operator*(double scalar) const {
    return {(int) round((double)x * scalar), (int) round((double)y * scalar)};
}

Vector2D Vector2D::operator-(const Vector2D& other) const {
    return {x - other.x, y - other.y};
}

// Chebyshev (L∞) norm
int Vector2D::chebyshevDistance(const Vector2D& other) const {
    return std::max(std::abs(x - other.x), std::abs(y - other.y));
}
