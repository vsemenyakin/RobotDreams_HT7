#include "MissionProcessor.hpp"
#include "ConfigLoader.hpp"
#include "TargetProvider.hpp"
#include "BallisticSolver.hpp"
#include "ConfigLoader.hpp"

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

	droneState = MissionProcessor::updateDrone(
		droneState,
		configLoader->getConfig().drone,
		configLoader->getConfig().simulation);
	
	//Place for the target attacking logic

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

	delete[] targetStates;
}

void MissionProcessor::clearMission() {
	stepIndex = 0;
	simulationTime = 0.f;

	delete[] targetStates;
	targetStates = nullptr;

	simulationStorage.reset();
}

void MissionProcessor::initMission() {
	droneState = DroneState{ configLoader->getConfig().drone };
	targetStates = new TargetState[targetProvider->getTargetCount()];
}

DroneState MissionProcessor::updateDrone(
	const DroneState& state,
	const DroneConfig& droneConfig,
	const SimulationConfig& simulationConfig)
{
	DroneState nextState = state;

	const float a = powf(droneConfig.attackSpeed, 2) / (2 * droneConfig.accelerationPath);

	nextState.state = state.state;
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

	if (std::abs(nextState.direction - state.targetAngle) < droneConfig.turnThreshold) {
		nextState.direction = state.targetAngle;

		//Stopped because drone can rotate only when stopped
		nextState.state = STOPPED;
	}

	const float velocityPerStep = nextState.velocity * simulationConfig.timeStep;
	nextState.position = state.position + Coord::createPolar(velocityPerStep, nextState.direction);

	return nextState;
}
