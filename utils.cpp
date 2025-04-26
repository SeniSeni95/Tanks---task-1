#include "utils.h"
using namespace std;

Vector2D Vector2D::operator+(const Vector2D& other) const {
    return {x + other.x, y + other.y};
}

Vector2D Vector2D::operator*(double scalar) const {
    return {x * scalar, y * scalar};
}

Vector2D Vector2D::operator-(const Vector2D& other) const {
    return {x - other.x, y - other.y};
}

// Chebyshev (L∞) norm
double Vector2D::chebyshevDistance(const Vector2D& other) const {
    return std::max(std::abs(x - other.x), std::abs(y - other.y));
}

// Finds the closest Chebyshev distance between a point and a parametric line
double chebyshevDistanceToLine(Vector2D& linePoint, const Vector2D& lineDir, const Vector2D& point, int n, int m) {
    if (point.chebyshevDistance(linePoint) < (point + lineDir).chebyshevDistance(linePoint)) {
        return -1; // The point is "behind" the line
    }

    double bestDist = std::numeric_limits<double>::max();
    while (true) {
        // Calculate the distance from the point to the line
        double dist = point.chebyshevDistance(linePoint);
        if (dist < bestDist) {
            bestDist = dist;
        }

        // Move along the line
        linePoint = linePoint + lineDir;

        // Check if the new point is out of bounds
        if (linePoint.x < 0 || linePoint.x >= n || linePoint.y < 0 || linePoint.y >= m) {
            break; // Exit the loop if out of bounds
        }
    }

    return bestDist; // Return the closest distance found
}

std::pair<int, int> rotate_4(int directionx, int directiony, std::string direction) {
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

std::pair<int, int> rotate_8(int directionx, int directiony, std::string direction) {
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