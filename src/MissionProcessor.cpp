#include "MissionProcessor.hpp"
#include "config/ConfigLoaderFabric.hpp"
#include "providers/TargetProviderFabric.hpp"
#include "solvers/BallisticSolverFabric.hpp"
#include "Utils.hpp"

#include <limits>
#include <assert.h>

///////////////////////////////////////////////////////////// State machine {{{

struct SimulationConfigs {
	const DroneConfig& droneConfig;
	const SimulationConfig& simulationConfig;
};

class IDroneState {
public:
    virtual ~IDroneState() = default;
 
    virtual std::unique_ptr<IDroneState>
        execute(DroneState& inoutDroneState, const SimulationConfigs& inConfigs) = 0;
 
    virtual const char* name() const = 0;
};

// --------------

class StoppedState : public IDroneState {
public:
    std::unique_ptr<IDroneState> execute(DroneState& inoutDroneState, const SimulationConfigs& inConfigs) override
	{
		//Drone stopped - no any changes
		// for dynamic properties are needed
		return nullptr;
	}

	const char* name() const override { return "STOPPED"; }
};

// -------------

class MovingState : public IDroneState {
public:
	virtual std::unique_ptr<IDroneState>
		execute(DroneState& inoutDroneState, const SimulationConfigs& inConfigs)
	{
		DroneState nextState = inoutDroneState;

		const float velocityPerStep = nextState.velocity * inConfigs.simulationConfig.timeStep;
		nextState.position = inoutDroneState.position + Coord::createPolar(velocityPerStep, nextState.direction);

		inoutDroneState = nextState;

		return nullptr;
	}

	const char* name() const override { return "MOVING"; }
};

class AcceleratingState : public IDroneState {
public:
    std::unique_ptr<IDroneState> execute(DroneState& inoutDroneState, const SimulationConfigs& inConfigs) override
	{
		DroneState nextState = inoutDroneState;

		nextState.velocity = inoutDroneState.velocity + inoutDroneState.acceleration * inConfigs.simulationConfig.timeStep;
		if (nextState.velocity >= inConfigs.droneConfig.attackSpeed) {
			nextState.velocity = inConfigs.droneConfig.attackSpeed;

			const float velocityPerStep = nextState.velocity * inConfigs.simulationConfig.timeStep;
			nextState.position = inoutDroneState.position + Coord::createPolar(velocityPerStep, nextState.direction);

			inoutDroneState = nextState;
			
			return std::make_unique<MovingState>();
		}

		const float velocityPerStep = nextState.velocity * inConfigs.simulationConfig.timeStep;
		nextState.position = inoutDroneState.position + Coord::createPolar(velocityPerStep, nextState.direction);

		inoutDroneState = nextState;

		return nullptr;
	}

	const char* name() const override { return "ACCELERATING"; }
};

// -------------

class DeceleratingState : public IDroneState {
public:
    std::unique_ptr<IDroneState> execute(DroneState& inoutDroneState, const SimulationConfigs& inConfigs) override
	{
		DroneState nextState = inoutDroneState;

		nextState.velocity = inoutDroneState.velocity - inoutDroneState.acceleration * inConfigs.simulationConfig.timeStep;
		if (nextState.velocity <= 0.f) {
			nextState.velocity = 0.f;

			const float velocityPerStep = nextState.velocity * inConfigs.simulationConfig.timeStep;
			nextState.position = inoutDroneState.position + Coord::createPolar(velocityPerStep, nextState.direction);

			inoutDroneState = nextState;

			return std::make_unique<StoppedState>();
		}

		const float velocityPerStep = nextState.velocity * inConfigs.simulationConfig.timeStep;
		nextState.position = inoutDroneState.position + Coord::createPolar(velocityPerStep, nextState.direction);

		inoutDroneState = nextState;

		return nullptr;
	}

	const char* name() const override { return "DECELERATING"; }
};

// -------------

class TurningPlusState : public IDroneState {
public:
    std::unique_ptr<IDroneState> execute(DroneState& inoutDroneState, const SimulationConfigs& inConfigs) override
	{
		DroneState nextState = inoutDroneState;

		nextState.direction = inoutDroneState.direction + inConfigs.droneConfig.angularSpeed;

		const float leftTurn = nextState.direction - inoutDroneState.targetAngle;
		if (std::abs(leftTurn) < inConfigs.droneConfig.turnThreshold ||
			std::abs(leftTurn) < inConfigs.droneConfig.angularSpeed)
		{
			nextState.direction = inoutDroneState.targetAngle;

			inoutDroneState = nextState;

			//Stopped because drone can rotate only when stopped
			return std::make_unique<StoppedState>();
		}

		inoutDroneState = nextState;

		return nullptr;
	}

	const char* name() const override { return "TURNING_PLUS"; }
};

// -------------

class TurningMinusState : public IDroneState {
public:
    std::unique_ptr<IDroneState> execute(DroneState& inoutDroneState, const SimulationConfigs& inConfigs) override
	{
		DroneState nextState = inoutDroneState;

		nextState.direction = inoutDroneState.direction - inConfigs.droneConfig.angularSpeed;

		const float leftTurn = nextState.direction - inoutDroneState.targetAngle;
		if (std::abs(leftTurn) < inConfigs.droneConfig.turnThreshold ||
			std::abs(leftTurn) < inConfigs.droneConfig.angularSpeed)
		{
			nextState.direction = inoutDroneState.targetAngle;

			inoutDroneState = nextState;

			//Stopped because drone can rotate only when stopped
			return std::make_unique<StoppedState>();
		}

		inoutDroneState = nextState;

		return nullptr;
	}

	const char* name() const override { return "TURNING_MINUS"; }
};

///////////////////////////////////////////////////////////// }}} State machine

MissionProcessor::MissionProcessor(
		const char* inAmmoConfigFileName,
		const char* inConfigFileName,
		const char* inTargetsFileName)
{
	configLoader = createLoader(LoaderType::FILE, inAmmoConfigFileName, inConfigFileName);
	assert(configLoader && "Failed to create config loader");

	targetProvider = createProvider(ProviderType::JSON, inTargetsFileName, configLoader->getConfig().targetArrayTimeStep);
	ballisticSolver = createSolver(SolverType::TABLE, "ballistic_table.txt");

	SM_droneState = std::make_unique<StoppedState>();

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
				SM_droneState = std::make_unique<TurningMinusState>();
			}
			else if (direction > droneState.direction) {
				droneState.targetAngle = direction;
				SM_droneState = std::make_unique<TurningPlusState>();
			}
			else {
				SM_droneState = std::make_unique<AcceleratingState>();

				if (distanceToAimingPosition <= horizontalDistance) {
					droneState.dropPoint = droneState.position;
				}
			}
		}
	}

	droneAIState.previousTargetState = currentTargetState;
	//}}}

	MissionProcessor::updateDrone(
		droneState,
		SM_droneState,
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

void MissionProcessor::changeSolver(std::unique_ptr<IBallisticSolver> newSolver) {
	ballisticSolver = std::move(newSolver);
}

void MissionProcessor::writeResults(const char* inFileName) {
	simulationStorage.writeToJSONFile(inFileName);
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

void MissionProcessor::updateDrone(
	DroneState& state,
	std::unique_ptr<IDroneState>& SM_droneState,
	const DroneConfig& droneConfig,
	const SimulationConfig& simulationConfig)
{
	std::unique_ptr<IDroneState> SM_nextDroneState = SM_droneState->execute(state, SimulationConfigs{ droneConfig, simulationConfig });
	if (SM_nextDroneState) {
		SM_droneState = std::move(SM_nextDroneState);
	}
}

MissionProcessor::~MissionProcessor() = default;
