#pragma once

#include "CommonTypes.hpp"

class SimulationStorage
{
public:
	SimulationStorage();

	void addState(const DroneState& inState);
	void reset();

	void writeToJSONFile(const char* inFileName) const;

	~SimulationStorage();

private:
	void resizeIfNeeded();

	static constexpr size_t statesReserveMultiplier = 10;

	DroneState* states = nullptr;
	size_t statesNumber = 0;
};
