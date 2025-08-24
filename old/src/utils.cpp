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


const int DIR_COUNT = 8;
const int dirX[DIR_COUNT] = {  0,  1,  1,  1,  0, -1, -1, -1 };
const int dirY[DIR_COUNT] = {  1,  1,  0, -1, -1, -1,  0,  1 };

std::pair<int, int> rotateDirection(int dirx, int diry, string direction) {
    int index = -1;
    for (int i = 0; i < DIR_COUNT; ++i) {
        if (dirX[i] == dirx && dirY[i] == diry) {
            index = i;
            break;
        }
    }

    if (index == -1) {
        return {dirx, diry}; // If the direction is not found, return the original direction
    }

    if (direction == "left") {
        index = (index + 1) % DIR_COUNT;
    } else if (direction == "right") {
        index = (index + DIR_COUNT - 1) % DIR_COUNT;
    } else {
        throw invalid_argument("Direction must be 'left' or 'right'.");
    }

    return {dirX[index], dirY[index]}; // Return the new direction vector
}

std::pair<int, int> rotate_4(int directionx, int directiony, std::string direction) {
    pair<int, int> rotateOnce = rotateDirection(directionx, directiony, direction);
    pair<int, int> rotateTwice = rotateDirection(rotateOnce.first, rotateOnce.second, direction);
    return {rotateTwice.first, rotateTwice.second}; // Return the new direction after two rotations
}

std::pair<int, int> rotate_8(int directionx, int directiony, std::string direction) {
    pair<int, int> rotate = rotateDirection(directionx, directiony, direction);
    return {rotate.first, rotate.second}; // Return the new direction after one rotation
}

std::string join(const std::vector<std::string>& vec, const std::string& delim) {
    std::ostringstream oss;
    for (size_t i = 0; i < vec.size(); ++i) {
        if (i > 0) oss << delim;
        oss << vec[i];
    }
    return oss.str();
}
