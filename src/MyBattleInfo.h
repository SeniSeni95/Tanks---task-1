#pragma once
#include "BattleInfo.h"
#include <vector>
#include <string>

enum class ObjectType {
    Tank,
    Mine,
    Shell
};

struct ObjectInfo {
    ObjectType type;
    int dx, dy; // Relative position from tank
};

class MyBattleInfo : public BattleInfo {
public:
    int shells = 0;
    int max_steps = 0;
    int x = 0, y = 0;
    std::vector<ObjectInfo> visibleObjects;

    ~MyBattleInfo() override = default;
};
