#pragma once

#include "CommonTypes.hpp"
#include "SimulationStorage.hpp"

#include <optional>
#include <vector>
#include <memory>

class IConfigLoader;
class ITargetProvider;
class IBallisticSolver;
class IDroneState;

class MissionProcessor {
private:
	static constexpr size_t maxSteps = 10000;

public:
	MissionProcessor(
		const char* inAmmoConfigFileName,
		const char* inConfigFileName,
		const char* inTargetsFileName);

	bool hasNext() const;

	void step();
	void reset();

	void changeSolver(std::unique_ptr<IBallisticSolver> newSolver);

	void writeResults(const char* inFileName);

	~MissionProcessor();

private:
	void clearMission();
	void initMission();

	static void updateDrone(
		DroneState& state,
		std::unique_ptr<IDroneState>& SM_droneState,
		const DroneConfig& droneConfig,
		const SimulationConfig& simulationConfig);

	std::unique_ptr<IConfigLoader> configLoader{ nullptr };
	std::unique_ptr<ITargetProvider> targetProvider{ nullptr };
	std::unique_ptr<IBallisticSolver> ballisticSolver{ nullptr };

	size_t stepIndex{ 0 };
	float simulationTime{ 0 };

	DroneState droneState{ };
	std::unique_ptr<IDroneState> SM_droneState{ };
	std::vector<TargetState> targetStates{ };

	struct DroneAIState
	{
		std::optional<TargetState> previousTargetState{ };
		std::optional<Coord> currentAimingPosition{ };
	};
	DroneAIState droneAIState{ };

	SimulationStorage simulationStorage{ };
};
