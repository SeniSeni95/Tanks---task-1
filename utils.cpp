#include <iostream>
#include <cmath>
#include <algorithm>
#include <limits>
#include "utils.h"
using namespace std;

struct Vector2D {
    double x, y;

    Vector2D operator+(const Vector2D& other) const {
        return {x + other.x, y + other.y};
    }

    Vector2D operator*(double scalar) const {
        return {x * scalar, y * scalar};
    }

    Vector2D operator-(const Vector2D& other) const {
        return {x - other.x, y - other.y};
    }

    // Chebyshev (L∞) norm
    double chebyshevDistance(const Vector2D& other) const {
        return std::max(std::abs(x - other.x), std::abs(y - other.y));
    }
};

// Finds the closest Chebyshev distance between a point and a parametric line
double chebyshevDistanceToLine(const Vector2D& linePoint, const Vector2D& lineDir, const Vector2D& point) {
    // Binary search for t in a reasonable range
    // (left is 0 because we want to only consider the line segment starting at linePoint, not the entire line)
    double left = 0, right = 1e6;
    double bestDist = std::numeric_limits<double>::max();

    for (int i = 0; i < 100; ++i) {  // Enough iterations for good precision
        double mid1 = left + (right - left) / 3.0;
        double mid2 = right - (right - left) / 3.0;

        Vector2D p1 = linePoint + lineDir * mid1;
        Vector2D p2 = linePoint + lineDir * mid2;

        double d1 = p1.chebyshevDistance(point);
        double d2 = p2.chebyshevDistance(point);

        bestDist = std::min(bestDist, std::min(d1, d2));

        if (d1 < d2)
            right = mid2;
        else
            left = mid1;
    }

    return bestDist;
}

std::pair<int, int> rotate_4(int directionx, int directiony, string direction) {
    // Calculate the angle in radians
    double degree = atan2(directiony, directionx) * (180.0 / M_PI); // Convert radians to degrees
    if (direction == "left") {
        degree += 90;
    } else if (direction == "right") {
        degree -= 90;
    }
    // Normalize the angle to the range [0, 360)
    if (degree < 0) {
        degree += 360;
    } else if (degree >= 360) {
        degree -= 360;
    }
    // Convert degrees back to radians for trigonometric functions
    double radian = degree * (M_PI / 180.0);
    // Update directionx and directiony using cos and sin
    directionx = round(cos(radian)); // Round to avoid floating-point precision issues
    directiony = round(sin(radian));
    
    return {directionx, directiony}; // Return the new direction
}

std::pair<int, int> rotate_8(int directionx, int directiony, string direction) {
        // Calculate the angle in radians
    double degree = atan2(directiony, directionx) * (180.0 / M_PI); // Convert radians to degrees
    if (direction == "left") {
        degree += 45;
    } else if (direction == "right") {
        degree -= 45;
    }
    // Normalize the angle to the range [0, 360)
    if (degree < 0) {
        degree += 360;
    } else if (degree >= 360) {
        degree -= 360;
    }
    // Convert degrees back to radians for trigonometric functions
    double radian = degree * (M_PI / 180.0);
    // Update directionx and directiony using cos and sin
    directionx = round(cos(radian)); // Round to avoid floating-point precision issues
    directiony = round(sin(radian));

    return {directionx, directiony}; // Return the new direction
}