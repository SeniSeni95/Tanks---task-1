#include "MyTankAlgorithm.h"
#include "MyBattleInfo.h"
#include <iostream>



MyTankAlgorithm::MyTankAlgorithm(int player_index, int tank_index)
    : player_index(player_index), tank_index(tank_index), currentInfo(nullptr)
{}

void MyTankAlgorithm::updateBattleInfo(BattleInfo& /*info*/) { }

ActionRequest MyTankAlgorithm::getAction() {
    std::string input;
    std::cout << "Enter action (fw, bw, r4l, r4r, r8l, r8r, shoot, skip, update): ";
    std::cin >> input;

    if (input == "fw") return ActionRequest::MoveForward;
    if (input == "bw") return ActionRequest::MoveBackward;
    if (input == "r4l") return ActionRequest::RotateLeft90;
    if (input == "r4r") return ActionRequest::RotateRight90;
    if (input == "r8l") return ActionRequest::RotateLeft45;
    if (input == "r8r") return ActionRequest::RotateRight45;
    if (input == "shoot") return ActionRequest::Shoot;
    if (input == "skip") return ActionRequest::DoNothing;
    if (input == "update") return ActionRequest::GetBattleInfo;

    std::cout << "Invalid input. Defaulting to skip." << std::endl;
    return ActionRequest::DoNothing;
}