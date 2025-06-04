#pragma once

#ifndef TANK_ALGORITHM_FACTORY_H
#define TANK_ALGORITHM_FACTORY_H

class TankAlgorithmFactory {
public:
	virtual ~TankAlgorithmFactory() {}
virtual unique_ptr<TankAlgorithm> create(
int player_index, int tank_index) const = 0;
};
#endif