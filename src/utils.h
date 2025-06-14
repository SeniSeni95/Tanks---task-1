#ifndef UTILS_H
#define UTILS_H
#include <sstream>
#include <cmath>
#include <algorithm>
#include <limits>
#include <string>
#include "Board.h"
#include "GameObject.h"
#include "Vector2D.h"

// Forward declarations
class game_board;
class cell;
class game_object;
class tank;
class shell;
class mine;
class wall;


// Finds the closest Chebyshev distance between a point and a parametric line
std::pair<int, int> chebyshevDistanceToLine(const Vector2D& linePoint, const Vector2D& lineDir, const Vector2D& point, game_board* board);
std::string join(const std::vector<std::string>& vec, const std::string& delim);
std::pair<int, int> rotate_4(int directionx, int directiony, std::string direction);
std::pair<int, int> rotate_8(int directionx, int directiony, std::string direction);

#endif // UTILS_H
