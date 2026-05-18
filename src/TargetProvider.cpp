#include "TargetProvider.hpp"
#include "JSONUtils.hpp"

#include "json.hpp"

#include <cassert>

using json = nlohmann::json;

// ---------------

class JSONConfigTargetProvider : public ITargetProvider {
public:
	JSONConfigTargetProvider(const char* inConfigFileName, const float inTargetArrayTimeStep);

	size_t getTargetCount() override;
	TargetState getTarget(const int inIndex, const float inSimulationTime) override;

private:
	struct TargetsConfig
	{
		static TargetsConfig createFromJSONFile(const char* inFileName);

	#ifdef DebugPrint
		void print() const;
	#endif

		~TargetsConfig();

		size_t targetCount{ 0 };
		size_t timeSteps{ 0 };
		Coord** targetsPositions{ nullptr };
	};

	TargetState getTargetStateAtTime(
		const TargetsConfig& inConfig,
		const size_t targetIndex,
		const float inSimulationTime,
		const float inArrayTimeStep);

	TargetsConfig config;
	float targetArrayTimeStep{ 0.f };
};

// ----------------

JSONConfigTargetProvider::JSONConfigTargetProvider(const char* inConfigFileName, const float inTargetArrayTimeStep) :
	config(TargetsConfig::createFromJSONFile(inConfigFileName)),
	targetArrayTimeStep(inTargetArrayTimeStep)
{
}

size_t JSONConfigTargetProvider::getTargetCount() {
	return config.targetCount;
}

TargetState JSONConfigTargetProvider::getTarget(const int inIndex, const float inSimulationTime) {
	assert(inIndex < config.targetCount);
	return getTargetStateAtTime(config, inIndex, inSimulationTime, targetArrayTimeStep);
}

JSONConfigTargetProvider::TargetsConfig JSONConfigTargetProvider::TargetsConfig::createFromJSONFile(const char* inFileName)
{
	TargetsConfig result;

	std::fstream inputFile{ inFileName };
	json jsonData = json::parse(inputFile);

	result.timeSteps = jsonData["timeSteps"];
	result.targetCount = jsonData["targetCount"];

	json targetsJSON = jsonData["targets"];
	const size_t targetsNum = targetsJSON.size();
	assert(targetsNum == result.targetCount);
	result.targetsPositions = new Coord*[targetsNum];
	for (size_t index = 0; index < targetsNum; ++index)
	{
		const json& targetObjectJSON = targetsJSON[index];
		const json& targetPositionsJSON = targetObjectJSON["positions"];

		size_t arraySize;
		readArrayFromJSON(result.targetsPositions[index], arraySize, targetPositionsJSON);
		assert(arraySize == result.timeSteps);
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

JSONConfigTargetProvider::TargetsConfig::~TargetsConfig()
{
	if (targetsPositions != nullptr)
	{
		for (size_t targetIndex = 0; targetIndex < targetCount; ++targetIndex)
		{
			assert(targetsPositions[targetIndex] != nullptr);
			delete[] targetsPositions[targetIndex];
		}

		delete[] targetsPositions;
	}
}

TargetState JSONConfigTargetProvider::getTargetStateAtTime(
	const TargetsConfig& inConfig,
	const size_t targetIndex,
	const float inSimulationTime,
	const float inArrayTimeStep)
{
	assert(targetIndex < inConfig.targetCount);

	TargetState result;

	int currentPositionIndex = static_cast<int>(floor(inSimulationTime / inArrayTimeStep)) % inConfig.timeSteps;
	int nextPositionIndex = (currentPositionIndex + 1) % inConfig.timeSteps;

	float frac = (inSimulationTime - currentPositionIndex * inArrayTimeStep) / inArrayTimeStep;

	const Coord currentPosition = inConfig.targetsPositions[targetIndex][currentPositionIndex];
	const Coord nextPosition = inConfig.targetsPositions[targetIndex][nextPositionIndex];

	result.position = currentPosition + (nextPosition - currentPosition) * frac;

	return result;
}

//Factory function

ITargetProvider* createJSONTargetProvider(const char* inConfigFileName, const float inTargetArrayTimeStep) {
	return new JSONConfigTargetProvider(inConfigFileName, inTargetArrayTimeStep);
}
