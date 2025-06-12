#include "utils.h"
#include <sstream>
using namespace std;
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Finds the closest Chebyshev distance between a point and a parametric line
std::pair<int, int> chebyshevDistanceToLine(const Vector2D& linePoint, const Vector2D& lineDir, const Vector2D& point, game_board* board) {
    if (point.x == linePoint.x && point.y == linePoint.y) {
        return {0, 0}; // The point is on the line
    }

    int n = board->n;
    int m = board->m;

    Vector2D newPoint = linePoint + lineDir;

    int bestTrajDist = std::numeric_limits<int>::max();
    int bestDist = std::numeric_limits<int>::max();

    int i = 0;
    while (true) {
        newPoint.x = (newPoint.x + n) % n; // Wrap around the x-coordinate
        newPoint.y = (newPoint.y + m) % m; // Wrap around the y-coordinate

        // Check if new point is blocked by a wall
        if (board->get_cell(newPoint.x, newPoint.y).has_Object()) {
            game_object* obj = board->get_cell(newPoint.x, newPoint.y).get_Object();
            if (obj->get_symbol() == '#') {
                break; // Stop if we hit a wall
            }
        }

        // Calculate the distance from the point to the line
        double dist = point.chebyshevDistance(newPoint);
        if (dist < bestTrajDist || (dist == bestTrajDist && i + 1 < bestDist)) {
            bestTrajDist = dist;
            bestDist = i + 1;
        }

        if ((linePoint.x == newPoint.x && linePoint.y == newPoint.y) || i > max(abs(n*lineDir.x), abs(m*lineDir.y))) {
            break; // Exit the loop if we are back at the starting point
        }

        // Move along the line
        newPoint = newPoint + lineDir;
        i++;
    }

    return {bestTrajDist, bestDist}; // Return the best distance and trajectory distance
}

std::pair<int, int> rotate_4(int directionx, int directiony, std::string direction) {
    // Calculate the angle in radians
    double degree = atan2(directionx, directiony) * (180.0 / M_PI); // Convert radians to degrees
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
    double degree = atan2(directionx, directiony) * (180.0 / M_PI); // Convert radians to degrees
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

std::string join(const std::vector<std::string>& vec, const std::string& delim) {
    std::ostringstream oss;
    for (size_t i = 0; i < vec.size(); ++i) {
        if (i > 0) oss << delim;
        oss << vec[i];
    }
    return oss.str();
}