#include "SimulationStorage.hpp"

#include <fstream>

void SimulationStorage::addState(const DroneState& inState)
{
	states.push_back(inState);
}

void SimulationStorage::reset()
{
	states.clear();
}

void SimulationStorage::writeToJSONFile(const std::string& inFileName) const
{
	json resultJSON{ };

	resultJSON["totalSteps"] = states.size();

	json stepsJSON = json::array();
	for (const DroneState& state : states)
	{
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
