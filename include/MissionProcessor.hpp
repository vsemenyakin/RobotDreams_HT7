#pragma once

#include "CommonTypes.hpp"
#include "SimulationStorage.hpp"

#include <optional>
#include <vector>

class IConfigLoader;
class ITargetProvider;
class IBallisticSolver;

class MissionProcessor {
private:
	static constexpr size_t maxSteps = 10;

public:
	MissionProcessor(
		const char* inAmmoConfigFileName,
		const char* inConfigFileName,
		const char* inTargetsFileName);

	bool hasNext() const;

	void step();
	void reset();

	void changeSolver(IBallisticSolver* newSolver);

	void writeResults(const char* inFileName);

	~MissionProcessor();

private:
	void clearMission();
	void initMission();

	static DroneState updateDrone(
		const DroneState& state,
		const DroneConfig& droneConfig,
		const SimulationConfig& simulationConfig);

	IConfigLoader* configLoader{ nullptr };
	ITargetProvider* targetProvider{ nullptr };
	IBallisticSolver* ballisticSolver{ nullptr };

	size_t stepIndex{ 0 };
	float simulationTime{ 0 };

	DroneState droneState{ };
	std::vector<TargetState> targetStates{ };

	struct DroneAIState
	{
		std::optional<TargetState> previousTargetState{ };
	};
	DroneAIState droneAIState{ };

	SimulationStorage simulationStorage{ };
};
