#include "CommonTypes.hpp"

#include <fstream>
#include <assert.h>

#ifdef DebugPrint
#include <iostream>
#endif

//-----------------------------------------------

#ifdef DebugPrint
void AmmoParams::print() const
{
	std::cout << "--- Ammo params ---" << std::endl;

	PrintField(name);
	PrintField(mass);
	PrintField(drag);
	PrintField(lift);
}
#endif

//-----------------------------------------------

Coord Coord::createPolar(const float inLength, const float inAngle)
{
	Coord result;

	result.x = inLength * std::cos(inAngle);
	result.y = inLength * std::sin(inAngle);

	return result;
}

Coord Coord::operator+(const Coord& other) const
{
	Coord result;

	result.x = x + other.x;
	result.y = y + other.y;

	return result;
}

Coord Coord::operator-(const Coord& other) const
{
	Coord result;

	result.x = x - other.x;
	result.y = y - other.y;

	return result;
}

Coord Coord::operator*(float s) const
{
	Coord result;

	result.x = x * s;
	result.y = y * s;

	return result;
}

Coord Coord::operator/(float s) const
{
	Coord result;

	result.x = x / s;
	result.y = y / s;

	return result;
}

bool Coord::operator==(const Coord& other) const
{
	return (x == other.x) && (y == other.y);
}

float Coord::length() const
{
	return std::sqrt(x * x + y * y);
}

float Coord::angle() const
{
	return std::atan2(y, x);
}

#ifdef DebugPrint
void Coord::print() const
{
	std::cout << "{" << x << ", " << y << "}" << std::endl;
}
#endif

//-----------------------------------------------

#ifdef DebugPrint
void DroneConfig::print() const
{
	std::cout << "--- Drone config ---" << std::endl;

	position.print();
	PrintField(altitude);
	PrintField(initialDirection);
	PrintField(attackSpeed);
	PrintField(accelerationPath);
	PrintField(angularSpeed);
	PrintField(turnThreshold);
}
#endif

//-----------------------------------------------

#ifdef DebugPrint
void SimulationConfig::print() const
{
	std::cout << "--- Simulation config ---" << std::endl;

	PrintField(timeStep);
	PrintField(hitRadius);
}
#endif

//-----------------------------------------------

Config Config::createFromJSONFile(const char* inFileName)
{
	Config result;

	std::fstream inputFile{ inFileName };
	json jsonData = json::parse(inputFile);

	result = jsonData;

	return result;
}

void Config::writeToJSONFile(const char* inFileName) const
{
	json outputJSON{ };
	outputJSON = *this;

	std::ofstream outputFile{ inFileName };
	outputFile << outputJSON.dump(2);
}

#ifdef DebugPrint
void Config::print() const
{
	std::cout << "--- Config ---" << std::endl;

	drone.print();
	PrintField(ammo);
	simulation.print();
	PrintField(targetArrayTimeStep);
}
#endif //DebugPrint

//-----------------------------------------------

const char* toString(const EDroneState inDroneState)
{
	switch (inDroneState)
	{
		case STOPPED:        return "STOPPED";
		case ACCELERATING:   return "ACCELERATING";
		case DECELERATING:   return "DECELERATING";
		case TURNING_PLUS:   return "TURNING_PLUS";
		case TURNING_MINUS:  return "TURNING_MINUS";
		case MOVING:         return "MOVING";
		default:             return "<unknown>";
	}
}

//-----------------------------------------------

DroneState::DroneState(const DroneConfig& inConfig)
{
	position = inConfig.position;
	direction = inConfig.initialDirection;

	velocity = 0.f;

	EDroneState state = STOPPED;
	targetAngle = 0.f;

	targetIndex = 0;
	dropPoint = {};
	aimPoint = {};
	predictedTarget = {};
}

//-----------------------------------------------

AmmoConfig AmmoConfig::createFromJSONFile(const char* inFileName)
{
	AmmoConfig result;

	std::fstream inputFile{ inFileName };
	const json jsonData = json::parse(inputFile);

	const size_t paramsCount = jsonData.size();
	for (size_t paramIndex = 0; paramIndex < paramsCount; ++paramIndex)
	{
		const AmmoParams ammoParam = jsonData[paramIndex];
		result.paramsMap[ammoParam.name] = ammoParam;
	}

	inputFile.close();

	return result;
}

const AmmoParams* AmmoConfig::getParams(const std::string& inParamsName) const
{
	auto iterator = paramsMap.find(inParamsName);
	return (iterator != paramsMap.end()) ? &iterator->second : nullptr;
}

#ifdef DebugPrint
void AmmoConfig::print() const
{
	std::cout << "--- Ammo config ---" << std::endl;

	for (const auto& pair : paramsMap)
	{
		pair.second.print();
	}
}
#endif
