#include "MissionProcessor.hpp"
#include "config/ConfigLoader.hpp"
#include "providers/TargetProvider.hpp"
#include "solvers/BallisticSolver.hpp"
#include "Utils.hpp"

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

		const Coord targetVelocity = (currentTargetState.position - droneAIState.previousTargetState->position) / config.targetArrayTimeStep;

		const Coord toTarget = currentTargetState.position - droneState.position;
		const float distanceToTarget = toTarget.length();
		
		float flightTime;
		float horizontalDistance;
		ballisticSolver->computeAmmoDrop(
			flightTime, horizontalDistance,
			droneConfig.attackSpeed,
			droneConfig.altitude,
			ammoParams);
		
		const float timeToDrop = (distanceToTarget - horizontalDistance) / droneConfig.attackSpeed;

		droneState.predictedTarget = currentTargetState.position + targetVelocity * timeToDrop;

		const float targetDirection = toTarget.angle();

		if (targetDirection < droneState.direction) {
			droneState.targetAngle = targetDirection;
			droneState.state = EDroneState::TURNING_MINUS;
		}
		else if (targetDirection > droneState.direction) {
			droneState.targetAngle = targetDirection;
			droneState.state = EDroneState::TURNING_PLUS;
		}
		else {
			droneState.state = EDroneState::ACCELERATING;

			if (equals(distanceToTarget, horizontalDistance, config.simulation.hitRadius)) {
				droneState.dropPoint = droneState.position;
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

	const float a = powf(droneConfig.attackSpeed, 2) / (2 * droneConfig.accelerationPath);

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
