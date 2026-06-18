#pragma once

#include "CommonTypes.hpp"

#include <memory>
#include <thread>
#include <atomic>
#include <mutex>
#include <variant>
#include <queue>

class IDroneState;

class DronePhysics {
public:
	DronePhysics(const DroneConfig& inDroneConfig, const SimulationConfig& inSimulationConfig);

	void start();
    void stop();

    DroneState getDroneState() const;

    //Drone control
    void setPredictedTarget(const Coord& inPredictedTarget);


    struct CommandTurn {
        float targetAngle{ 0.f };
    };

    struct CommandAccelerate {
    };

    struct CommandDrop {
    };

    using Command = std::variant<
        CommandTurn,
        CommandAccelerate,
        CommandDrop>;

    void addCommand(const Command& inCommand);

    ~DronePhysics();

private:
    void threadEnterPoint();

	void tick();

	DroneConfig droneConfig;
	SimulationConfig simulationConfig;

	DroneState droneState{ };
    mutable std::mutex droneStateMutex;
	
    std::unique_ptr<IDroneState> SM_droneState{ };

    std::thread thread;
    std::atomic<bool> isRunning{ false };

    std::queue<Command> commandQueue;
    std::mutex commandQueueMutex;
};
