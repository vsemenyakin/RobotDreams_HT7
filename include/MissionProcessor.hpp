#pragma once

#include "CommonTypes.hpp"
#include "SimulationStorage.hpp"

#include <optional>
#include <vector>
#include <memory>
#include <string>

class IConfigLoader;
class ITargetProvider;
class IBallisticSolver;
class DronePhysics;

class MissionProcessor {
private:
	static constexpr size_t maxSteps = 10000;

public:
	MissionProcessor(
		const std::string& inAmmoConfigFileName,
		const std::string& inConfigFileName,
		const std::string& inTargetsFileName);

	void start();

	void changeSolver(std::unique_ptr<IBallisticSolver> newSolver);

	void writeResults(const std::string& inFileName);

	~MissionProcessor();

private:
	void step();
	bool hasNext() const;

	std::unique_ptr<IConfigLoader> configLoader{ nullptr };
	std::unique_ptr<ITargetProvider> targetProvider{ nullptr };
	std::unique_ptr<IBallisticSolver> ballisticSolver{ nullptr };
	std::unique_ptr<DronePhysics> dronePhysics{ nullptr };

	size_t stepIndex{ 0 };
	float simulationTime{ 0 };

	struct DroneAIState
	{
		std::optional<TargetState> previousTargetState{ };
		std::optional<Coord> currentAimingPosition{ };
	};
	DroneAIState droneAIState{ };

	SimulationStorage simulationStorage{ };
};
