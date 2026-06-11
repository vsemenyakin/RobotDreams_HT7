#pragma once

#include "CommonTypes.hpp"

#include <vector>
#include <string>

class SimulationStorage
{
public:
	SimulationStorage() = default;

	void addState(const DroneState& inState);
	void reset();

	void writeToJSONFile(const std::string& inFileName) const;

private:
	std::vector<DroneState> states;
};
