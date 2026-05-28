#include "MissionProcessor.hpp"
#include "config/ConfigLoaderFabric.hpp"
#include "providers/TargetProviderFabric.hpp"
#include "solvers/BallisticSolverFabric.hpp"
#include "Utils.hpp"

#include <limits>

MissionProcessor::MissionProcessor(
		const char* inAmmoConfigFileName,
		const char* inConfigFileName,
		const char* inTargetsFileName)
{
	configLoader = createLoader(LoaderType::FILE, inAmmoConfigFileName, inConfigFileName);
	targetProvider = createProvider(ProviderType::JSON, inTargetsFileName, configLoader->getConfig().targetArrayTimeStep);
	ballisticSolver = createSolver(SolverType::ANALYTICAL);

	initMission();
}

bool MissionProcessor::hasNext() const {
	return (stepIndex < MissionProcessor::maxSteps);
}

float getTimeOfMovement(const float distance, const float initialVelocity, const float acceleration) {
	if (equals(acceleration, 0.f)) {
		return distance / initialVelocity;
	}

	return -initialVelocity / acceleration + std::sqrt(powf(initialVelocity, 2) + 2 * acceleration * distance) / acceleration;
}

float predictTimeToTarget(const Coord& targetPosition, const DroneState& droneState, const float AmmoFlightDistance) {
	const Coord toTarget = targetPosition - droneState.position;
	const float distanceToTarget = toTarget.length();
	const float angleToTarget = toTarget.angle();

	const float deltaAngle = std::abs(angleToTarget - droneState.direction);
	const float timeForRotation = deltaAngle / droneState.angularSpeed;

	const float distanceToDrop = distanceToTarget - AmmoFlightDistance;
	const float timeForDirectMove = getTimeOfMovement(distanceToDrop, droneState.velocity, droneState.acceleration);

	return timeForRotation + timeForDirectMove;
}

void MissionProcessor::step() {
	for (size_t targetIndex = 0; targetIndex < targetProvider->getTargetCount(); ++targetIndex)
	{
		targetStates[targetIndex] = targetProvider->getTarget(targetIndex, simulationTime);
	}

	//Place for drone "brain" logic {{{
	const Config& config = configLoader->getConfig();
	const DroneConfig& droneConfig = config.drone;
	const AmmoParams& ammoParams = *configLoader->getAmmoConfig().getParams(config.ammo);

	const TargetState& currentTargetState = targetStates[droneState.targetIndex];	

	if (droneAIState.previousTargetState.has_value()) {

		float flightTime;
		float horizontalDistance;
		ballisticSolver->computeAmmoDrop(
			flightTime, horizontalDistance,
			droneConfig.attackSpeed,
			droneConfig.altitude,
			ammoParams);

		if (!droneAIState.currentAimingPosition.has_value()) {

			const Coord targetVelocity = (currentTargetState.position - droneAIState.previousTargetState->position) / config.targetArrayTimeStep;

			Coord predictedTargetPosition = currentTargetState.position;
			const float targetSearchingTimeStep = configLoader->getConfig().simulation.timeStep;
			float lastDistanceBetweenRealAndPredicted = std::numeric_limits<float>::max();
			do
			{
				const float predictedTimeToTarget = predictTimeToTarget(predictedTargetPosition, droneState, horizontalDistance);
				Coord realTargetPositionAtPredictedTime = currentTargetState.position + targetVelocity * predictedTimeToTarget;

				Coord toTarget = realTargetPositionAtPredictedTime - predictedTargetPosition;
				const float distanceBetweenRealAndPredicted = toTarget.length();

				if (distanceBetweenRealAndPredicted > lastDistanceBetweenRealAndPredicted) {
					break;
				}

				lastDistanceBetweenRealAndPredicted = distanceBetweenRealAndPredicted;
				predictedTargetPosition = predictedTargetPosition + targetVelocity * targetSearchingTimeStep;
			} while (true);

			droneAIState.currentAimingPosition = predictedTargetPosition;

			droneState.predictedTarget = predictedTargetPosition;
		} else {
			const Coord toAimingPosition = droneAIState.currentAimingPosition.value() - droneState.position;
			const float distanceToAimingPosition = toAimingPosition.length();

			const float direction = toAimingPosition.angle();

			if (direction < droneState.direction) {
				droneState.targetAngle = direction;
				droneState.state = EDroneState::TURNING_MINUS;
			}
			else if (direction > droneState.direction) {
				droneState.targetAngle = direction;
				droneState.state = EDroneState::TURNING_PLUS;
			}
			else {
				droneState.state = EDroneState::ACCELERATING;

				if (distanceToAimingPosition <= horizontalDistance) {
					droneState.dropPoint = droneState.position;
				}
			}
		}
	}

	droneAIState.previousTargetState = currentTargetState;
	//}}}

	droneState = MissionProcessor::updateDrone(
		droneState,
		configLoader->getConfig().drone,
		configLoader->getConfig().simulation);

	simulationStorage.addState(droneState);

	simulationTime += configLoader->getConfig().simulation.timeStep;
	++stepIndex;
}

void MissionProcessor::reset() {
	clearMission();
	initMission();
}

void MissionProcessor::changeSolver(IBallisticSolver* newSolver) {
	delete ballisticSolver;
	
	ballisticSolver = newSolver;
}

void MissionProcessor::writeResults(const char* inFileName) {
	simulationStorage.writeToJSONFile(inFileName);
}

MissionProcessor::~MissionProcessor() {
	delete configLoader;
	delete targetProvider;
	delete ballisticSolver;
}

void MissionProcessor::clearMission() {
	stepIndex = 0;
	simulationTime = 0.f;

	targetStates.clear();

	simulationStorage.reset();
}

void MissionProcessor::initMission() {
	droneState = DroneState{ configLoader->getConfig().drone };

	targetStates.resize(targetProvider->getTargetCount());
}

DroneState MissionProcessor::updateDrone(
	const DroneState& state,
	const DroneConfig& droneConfig,
	const SimulationConfig& simulationConfig)
{
	DroneState nextState = state;

	const float a = state.acceleration;

	//Here state - is like "target" state
	switch (state.state)
	{
	case STOPPED:
		//Drone stopped - no any changes
		// for dynamic properties are needed
		break;

	case ACCELERATING:
		nextState.velocity = state.velocity + a * simulationConfig.timeStep;
		if (nextState.velocity >= droneConfig.attackSpeed) {
			nextState.velocity = droneConfig.attackSpeed;
			nextState.state = MOVING;
		}
		break;

	case DECELERATING:
		nextState.velocity = state.velocity - a * simulationConfig.timeStep;
		if (nextState.velocity <= 0.f) {
			nextState.velocity = 0.f;
			nextState.state = STOPPED;
		}
		break;

	case TURNING_PLUS:
		nextState.direction = state.direction + droneConfig.angularSpeed;
		break;

	case TURNING_MINUS:
		nextState.direction = state.direction - droneConfig.angularSpeed;
		break;

	case MOVING:
		//Drone stopped - no any changes
		// for dynamic properties are needed
		break;
	}

	if (state.state == TURNING_PLUS || state.state == TURNING_MINUS) {
		const float leftTurn = nextState.direction - state.targetAngle;
		if (std::abs(leftTurn) < droneConfig.turnThreshold ||
			std::abs(leftTurn) < droneConfig.angularSpeed)
		{
			nextState.direction = state.targetAngle;

			//Stopped because drone can rotate only when stopped
			nextState.state = STOPPED;
		}
	}

	const float velocityPerStep = nextState.velocity * simulationConfig.timeStep;
	nextState.position = state.position + Coord::createPolar(velocityPerStep, nextState.direction);

	return nextState;
}
