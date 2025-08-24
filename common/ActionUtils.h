#pragma once
#include <string>
#include "ActionRequest.h"

// Convert ActionRequest enum -> short command string
inline std::string actionToString(ActionRequest action) {
    switch (action) {
        case ActionRequest::MoveForward:   return "fw";
        case ActionRequest::MoveBackward:  return "bw";
        case ActionRequest::RotateLeft90:  return "r4l";
        case ActionRequest::RotateRight90: return "r4r";
        case ActionRequest::RotateLeft45:  return "r8l";
        case ActionRequest::RotateRight45: return "r8r";
        case ActionRequest::Shoot:         return "shoot";
        case ActionRequest::GetBattleInfo: return "update";
        case ActionRequest::DoNothing:     return "skip";
    }
    return "skip"; // fallback
}

// Convert short command string -> ActionRequest enum
inline ActionRequest stringToAction(const std::string& str) {
    if (str == "fw")     return ActionRequest::MoveForward;
    if (str == "bw")     return ActionRequest::MoveBackward;
    if (str == "r4l")    return ActionRequest::RotateLeft90;
    if (str == "r4r")    return ActionRequest::RotateRight90;
    if (str == "r8l")    return ActionRequest::RotateLeft45;
    if (str == "r8r")    return ActionRequest::RotateRight45;
    if (str == "shoot")  return ActionRequest::Shoot;
    if (str == "update") return ActionRequest::GetBattleInfo;
    if (str == "skip")   return ActionRequest::DoNothing;
    return ActionRequest::DoNothing;
}
