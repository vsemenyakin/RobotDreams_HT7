#include "providers/JSONConfigTargetProvider.hpp"

#include "json.hpp"

#include <fstream>
#include <vector>
#include <cassert>
#include <string>
#include <cmath>

#include <thread>
#include <mutex>
#include <atomic>

using json = nlohmann::json;

// ---------------

class JSONConfigTargetProvider : public ITargetProvider {
public:
	JSONConfigTargetProvider(
		const std::string& inConfigFileName,
		const float inTargetArrayTimeStep,
		const float inTickDeltaTime);

	size_t getTargetCount() const override;
	TargetState getTarget(const int inIndex) const override;

	bool isThreadReady() const override;
    void start() override;
    void stop() override;

private:
	struct TargetsConfig
	{
		static TargetsConfig createFromJSONFile(const std::string& inFileName);

	#ifdef DebugPrint
		void print() const;
	#endif

		size_t getTargetCount() const { return targetsPositions.size(); }
		size_t getTimeSteps() const { return targetsPositions[0].size(); }

		std::vector<std::vector<Coord>> targetsPositions{ };
	};

	void threadEnterPoint();
	void tick();

	TargetState getTargetStateAtTime(
		const TargetsConfig& inConfig,
		const size_t targetIndex,
		const float inSimulationTime,
		const float inArrayTimeStep);

	TargetsConfig config;
	float targetArrayTimeStep{ 0.f }; // Time between target control points
	float tickDeltaTime{ 0.f };
	float simulationTime{ 0.f };

	std::vector<TargetState> targetStates;
	mutable std::mutex targetStatesMutex;

	enum class State {
		ThreadNotCreated,
		ThreadReady,
		Started,
		Stopped
	};

	std::atomic<State> state{ State::ThreadNotCreated };

	std::thread thread;
};

// ----------------

JSONConfigTargetProvider::JSONConfigTargetProvider(
	const std::string& inConfigFileName,
	const float inTargetArrayTimeStep,
	const float inTickDeltaTime)
	:
	config(TargetsConfig::createFromJSONFile(inConfigFileName)),
	targetArrayTimeStep(inTargetArrayTimeStep),
	tickDeltaTime(inTickDeltaTime)
{
	//Make targets state array be big enough to contain all targets states
	targetStates.resize(getTargetCount());

	thread = std::thread{ &JSONConfigTargetProvider::threadEnterPoint, this };
}

size_t JSONConfigTargetProvider::getTargetCount() const {
	return config.getTargetCount();
}

TargetState JSONConfigTargetProvider::getTarget(const int inIndex) const {

	TargetState result;
	{
		std::lock_guard<std::mutex> guard(targetStatesMutex);

		assert(inIndex < getTargetCount());
		result = targetStates[inIndex];
	}

	return result;
}

bool JSONConfigTargetProvider::isThreadReady() const
{
	return (state != State::ThreadNotCreated);
}

void JSONConfigTargetProvider::start()
{
	state = State::Started;
}

void JSONConfigTargetProvider::stop()
{
	state = State::Stopped;
	thread.join();
}

JSONConfigTargetProvider::TargetsConfig JSONConfigTargetProvider::TargetsConfig::createFromJSONFile(const std::string& inFileName)
{
	TargetsConfig result;

	std::fstream inputFile{ inFileName };
	json jsonData = json::parse(inputFile);

	const size_t targetsCountForTest = jsonData["targetCount"];
	const size_t timeStepsForTest = jsonData["timeSteps"];

	json targetsJSON = jsonData["targets"];
	const size_t targetsCount = targetsJSON.size();
	assert(targetsCount > 0 && targetsCount == targetsCountForTest);
	for (size_t index = 0; index < targetsCount; ++index)
	{
		const json& targetObjectJSON = targetsJSON[index];
		const json& targetPositionsJSON = targetObjectJSON["positions"];

		std::vector<Coord> targetPositions;
		const size_t timeSteps = targetPositionsJSON.size();
		assert(timeSteps > 0 && timeSteps == timeStepsForTest);
		for (size_t positionIndex = 0; positionIndex < timeSteps; ++positionIndex)
		{
			const Coord position = targetPositionsJSON[positionIndex];
			targetPositions.push_back(position);
		}
		result.targetsPositions.push_back(targetPositions);
	}

	return result;
}

#ifdef DebugPrint
void ConfigTargetProvider::TargetsConfig::print() const
{
	std::cout << "--- Targets config ---" << std::endl;

	PrintField(targetCount);
	PrintField(timeSteps);

	for (size_t targetIndex = 0; targetIndex < targetCount; ++targetIndex)
	{
		std::cout << "Target [" << targetIndex << "]:" << std::endl;
		
		Coord* targetPositions = targetsPositions[targetIndex];
		for (size_t positionIndex = 0; positionIndex < timeSteps; ++positionIndex)
		{
			targetPositions[positionIndex].print();
		}
	}
}
#endif //DebugPrint

void JSONConfigTargetProvider::threadEnterPoint()
{
	state = State::ThreadReady;

	while (state != State::Stopped) {
		if (state == State::Started) {
			tick();
		}
	}
}

void JSONConfigTargetProvider::tick()
{
	std::lock_guard<std::mutex> guard(targetStatesMutex);

	simulationTime += tickDeltaTime;

	for (size_t targetIndex = 0; targetIndex < getTargetCount(); ++targetIndex)
	{
		targetStates[targetIndex] = getTargetStateAtTime(
			config, targetIndex, simulationTime, targetArrayTimeStep);
	}
}

TargetState JSONConfigTargetProvider::getTargetStateAtTime(
	const TargetsConfig& inConfig,
	const size_t targetIndex,
	const float inSimulationTime,
	const float inArrayTimeStep)
{
	TargetState result;

	const size_t timeSteps = inConfig.getTimeSteps();
	int currentPositionIndex = static_cast<int>(floor(inSimulationTime / inArrayTimeStep)) % timeSteps;
	int nextPositionIndex = (currentPositionIndex + 1) % timeSteps;

	const float simulationTimeNormalized = std::fmod(inSimulationTime, timeSteps * inArrayTimeStep);
	const float frac = (simulationTimeNormalized - currentPositionIndex * inArrayTimeStep) / inArrayTimeStep;

	const Coord currentPosition = inConfig.targetsPositions[targetIndex][currentPositionIndex];
	const Coord nextPosition = inConfig.targetsPositions[targetIndex][nextPositionIndex];

	result.position = currentPosition + (nextPosition - currentPosition) * frac;
	result.velocity = (nextPosition - currentPosition) / inArrayTimeStep;

	return result;
}

//Factory function

std::unique_ptr<ITargetProvider> createJSONTargetProvider(const std::string& inConfigFileName, const float inTargetArrayTimeStep, const float inTickDeltaTime) {
	return std::make_unique<JSONConfigTargetProvider>(inConfigFileName, inTargetArrayTimeStep, inTickDeltaTime);
}
