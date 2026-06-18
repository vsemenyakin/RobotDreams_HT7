#include "DronePhysics.hpp"

#include <iostream>

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

        std::cout << "MovingState: position = (" << inoutDroneState.position.x << ", " << inoutDroneState.position.y << ")" << std::endl;

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

DronePhysics::DronePhysics(const DroneConfig& inDroneConfig, const SimulationConfig& inSimulationConfig) :
    droneConfig(inDroneConfig),
    simulationConfig(inSimulationConfig),
    SM_droneState(std::make_unique<StoppedState>())
{
    thread = std::thread{ &DronePhysics::threadEnterPoint, this };

    droneState = DroneState{ inDroneConfig };
}

void DronePhysics::start() {
    isRunning = true;
}

void DronePhysics::stop() {
    isRunning = false;

    if (thread.joinable()) {
        thread.join();
    }
}

DroneState DronePhysics::getDroneState() const
{
    DroneState result;
    {
        std::lock_guard<std::mutex> guard{ droneStateMutex };
        result = droneState;
    }

    return result;
}

void DronePhysics::setPredictedTarget(const Coord& inPredictedTarget) {
    std::lock_guard<std::mutex> guard{ droneStateMutex };
    droneState.predictedTarget = inPredictedTarget;
}

void DronePhysics::addCommand(const Command& inCommand) {
    std::lock_guard<std::mutex> guard{ commandQueueMutex };
    commandQueue.push(inCommand);
}

void DronePhysics::threadEnterPoint() {
    while (isRunning) {
        tick();
    }
}

void DronePhysics::tick() {

    std::lock_guard<std::mutex> guard{ droneStateMutex };

    {
        //TODO: IMPLEMENT!
        std::lock_guard<std::mutex> commandGuard{ commandQueueMutex };
        while (!commandQueue.empty()) {
            const Command& command = commandQueue.front();

            if (const auto* turnCommand = std::get_if<CommandTurn>(&command)) {
                if (turnCommand->targetAngle < droneState.direction) {
                    droneState.targetAngle = turnCommand->targetAngle;
                    SM_droneState = std::make_unique<TurningMinusState>();
                }
                else if (turnCommand->targetAngle > droneState.direction) {
                    droneState.targetAngle = turnCommand->targetAngle;
                    SM_droneState = std::make_unique<TurningPlusState>();
                }
            } else if (const auto* accelerateCommand = std::get_if<CommandAccelerate>(&command)) {
                SM_droneState = std::make_unique<AcceleratingState>();
            } else if (const auto* dropCommand = std::get_if<CommandDrop>(&command)) {
                droneState.dropPoint = droneState.position;
            }

            commandQueue.pop();
        }
    }

    std::unique_ptr<IDroneState> SM_nextDroneState = SM_droneState->execute(droneState, SimulationConfigs{ droneConfig, simulationConfig });
    if (SM_nextDroneState) {
        SM_droneState = std::move(SM_nextDroneState);
    }
}

DronePhysics::~DronePhysics() = default;
