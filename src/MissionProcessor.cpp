#include "MissionProcessor.hpp"
#include "config/ConfigLoaderFactory.hpp"
#include "providers/TargetProviderFactory.hpp"
#include "solvers/BallisticSolverFactory.hpp"
#include "DronePhysics.hpp"
#include "Utils.hpp"

#include <limits>
#include <assert.h>
#include <iostream>

MissionProcessor::MissionProcessor(
		const std::string& inAmmoConfigFileName,
		const std::string& inConfigFileName,
		const std::string& inTargetsFileName)
{
	configLoader = createLoader(LoaderType::FILE, inAmmoConfigFileName, inConfigFileName);
	assert(configLoader && "Failed to create config loader");

	targetProvider = createProvider(ProviderType::JSON,
		inTargetsFileName,
		configLoader->getConfig().targetArrayTimeStep,
		configLoader->getConfig().simulation.timeStep);
	assert(targetProvider && "Failed to create target provider");

	ballisticSolver = createSolver(SolverType::TABLE, std::string{ "ballistic_table.txt" });
	assert(ballisticSolver && "Failed to create ballistic solver");

	dronePhysics = std::make_unique<DronePhysics>(
		configLoader->getConfig().drone, configLoader->getConfig().simulation);
}

float getTimeOfMovement(const float distance, const float initialVelocity, const float acceleration) {
	if (equals(acceleration, 0.f)) {
		return distance / initialVelocity;
	}

	return -initialVelocity / acceleration + std::sqrt(powf(initialVelocity, 2) + 2 * acceleration * distance) / acceleration;
}

std::optional<float> predictTimeToTarget(const Coord& targetPosition, const DroneState& droneState, const float AmmoFlightDistance) {
	const Coord toTarget = targetPosition - droneState.position;
	const float distanceToTarget = toTarget.length();
	const float angleToTarget = toTarget.angle();

	const float deltaAngle = std::abs(angleToTarget - droneState.direction);
	const float timeForRotation = deltaAngle / droneState.angularSpeed;

	const float distanceToDrop = distanceToTarget - AmmoFlightDistance;
	if (distanceToDrop <= 0.f) {
		return { };
	}

	const float timeForDirectMove = getTimeOfMovement(distanceToDrop, droneState.velocity, droneState.acceleration);

	return { timeForRotation + timeForDirectMove };
}

void MissionProcessor::step() {

	//Place for drone "brain" logic {{{
	const Config& config = configLoader->getConfig();
	const DroneConfig& droneConfig = config.drone;
	const AmmoParams& ammoParams = *configLoader->getAmmoConfig().getParams(config.ammo);
	
	const DroneState droneState = dronePhysics->getDroneState();
	const TargetState& currentTargetState = targetProvider->getTarget(droneState.targetIndex);	

	if (droneAIState.previousTargetState.has_value()) {

		float flightTime;
		float horizontalDistance;
		ballisticSolver->computeAmmoDrop(
			flightTime, horizontalDistance,
			droneConfig.attackSpeed,
			droneConfig.altitude,
			ammoParams);

		if (!droneAIState.currentAimingPosition.has_value()) {

			const Coord targetVelocity = currentTargetState.velocity;

			Coord predictedTargetPosition = currentTargetState.position;
			const float targetSearchingTimeStep = configLoader->getConfig().simulation.timeStep;
			float lastDistanceBetweenRealAndPredicted = std::numeric_limits<float>::max();

			bool foundPoint = false;
			do
			{
				const std::optional<float> predictedTimeToTarget = predictTimeToTarget(predictedTargetPosition, droneState, horizontalDistance);
				if (!predictedTimeToTarget.has_value()) {
					foundPoint = false;
					break;
				}

				Coord realTargetPositionAtPredictedTime = currentTargetState.position + targetVelocity * predictedTimeToTarget.value();

				Coord toTarget = realTargetPositionAtPredictedTime - predictedTargetPosition;
				const float distanceBetweenRealAndPredicted = toTarget.length();

				if (distanceBetweenRealAndPredicted > lastDistanceBetweenRealAndPredicted) {
					foundPoint = true;
					break;
				}

				lastDistanceBetweenRealAndPredicted = distanceBetweenRealAndPredicted;
				predictedTargetPosition = predictedTargetPosition + targetVelocity * targetSearchingTimeStep;
			} while (true);

			if (foundPoint) {
				droneAIState.currentAimingPosition = predictedTargetPosition;
				dronePhysics->setPredictedTarget(predictedTargetPosition);
			}
		} else {
			const Coord toAimingPosition = droneAIState.currentAimingPosition.value() - droneState.position;
			const float distanceToAimingPosition = toAimingPosition.length();

			const float direction = toAimingPosition.angle();

			if (direction != droneState.direction) {
				dronePhysics->addCommand(DronePhysics::CommandTurn{ direction });
			} else {
				dronePhysics->addCommand(DronePhysics::CommandAccelerate{ });

				if (distanceToAimingPosition <= horizontalDistance) {
					dronePhysics->addCommand(DronePhysics::CommandDrop{ });
				}
			}
		}
	}

	droneAIState.previousTargetState = currentTargetState;
	//}}}

	simulationStorage.addState(droneState);

	simulationTime += configLoader->getConfig().simulation.timeStep;
	++stepIndex;

	std::cout << "======================================= Step: " << stepIndex << std::endl;
}

bool MissionProcessor::hasNext() const {
	return (stepIndex < MissionProcessor::maxSteps);
}

void MissionProcessor::start() {
	assert(targetProvider->isThreadReady() && "Config loader is not initialized");

	targetProvider->start();
	dronePhysics->start();

	while (hasNext()) {
		step();
	}

	targetProvider->stop();
	dronePhysics->stop();
}

void MissionProcessor::changeSolver(std::unique_ptr<IBallisticSolver> newSolver) {
	ballisticSolver = std::move(newSolver);
}

void MissionProcessor::writeResults(const std::string& inFileName) {
	simulationStorage.writeToJSONFile(inFileName);
}

MissionProcessor::~MissionProcessor() = default;
