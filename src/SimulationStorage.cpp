#include "SimulationStorage.hpp"

SimulationStorage::SimulationStorage()
{
	states = new DroneState[statesReserveMultiplier];
}

void SimulationStorage::addState(const DroneState& inState)
{
	resizeIfNeeded();

	states[statesNumber] = inState;

	++statesNumber;
}

void SimulationStorage::reset()
{
	delete[] states;
	
	states = new DroneState[statesReserveMultiplier];
	statesNumber = 0;
}

void SimulationStorage::writeToJSONFile(const char* inFileName) const
{
	json resultJSON{ };

	resultJSON["totalSteps"] = statesNumber;

	json stepsJSON = json::array();
	for (size_t stateIndex = 0; stateIndex < statesNumber; ++stateIndex)
	{
		const DroneState& state = states[stateIndex];

		json stepJSON = json::object();
		stepJSON["position"] = state.position;
		stepJSON["direction"] = state.direction;
		stepJSON["state"] = state.state;
		stepJSON["targetIndex"] = state.targetIndex;
		stepJSON["dropPoint"] = state.dropPoint;
		stepJSON["aimPoint"] = state.aimPoint;
		stepJSON["predictedTarget"] = state.predictedTarget;

		stepsJSON.push_back(stepJSON);
	}

	resultJSON["steps"] = stepsJSON;

	std::ofstream outputFile{ inFileName };
	outputFile << resultJSON.dump(2);
}

SimulationStorage::~SimulationStorage()
{
	delete[] states;
}

void SimulationStorage::resizeIfNeeded()
{
	const size_t freeSpaceLeft = statesNumber % statesReserveMultiplier;
	if (freeSpaceLeft != 0)
		return;

	DroneState* newStates = new DroneState[statesNumber + statesReserveMultiplier];
	for (size_t stateIndex = 0; stateIndex < statesNumber; ++stateIndex)
	{
		newStates[stateIndex] = states[stateIndex];
	}

	delete[] states;
	states = newStates;
}
