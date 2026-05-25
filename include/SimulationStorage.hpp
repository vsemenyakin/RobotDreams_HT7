#pragma once

#include "CommonTypes.hpp"

#include <vector>

class SimulationStorage
{
public:
	SimulationStorage() = default;

	void addState(const DroneState& inState);
	void reset();

	void writeToJSONFile(const char* inFileName) const;

private:
	std::vector<DroneState> states;
};
