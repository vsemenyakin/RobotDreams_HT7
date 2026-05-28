#pragma once

#include "json.hpp"

#include <map>
#include <string>

using json = nlohmann::json;

//-----------------------------------------------

struct AmmoParams
{
	bool isValid() const
	{
		return name.length() > 0;
	}

#ifdef DebugPrint
	void print() const;
#endif

	std::string name{ };
	float mass{ 0.f };
	float drag{ 0.f };
	float lift{ 0.f };
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(AmmoParams, name, mass, drag, lift)

//-----------------------------------------------

struct Coord
{
	static Coord createPolar(const float inLength, const float inAngle);

	Coord operator+(const Coord& other) const;
	Coord operator-(const Coord& other) const;

	Coord operator*(float s) const;
	Coord operator/(float s) const;

	bool operator==(const Coord& other) const;
	
	float length() const;
	float angle() const;

#ifdef DebugPrint
	void print() const;
#endif

	float x;
	float y;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Coord, x, y)

//-----------------------------------------------

struct DroneConfig
{
	Coord position{ };
	float altitude{ 0.f };
	float initialDirection{ 0.f };
	float attackSpeed{ 0.f };
	float accelerationPath{ 0.f };
	float angularSpeed{ 0.f };
	float turnThreshold{ 0.f };

#ifdef DebugPrint
	void print() const;
#endif
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(DroneConfig,
	position,
	altitude,
	initialDirection,
	attackSpeed,
	accelerationPath,
	angularSpeed,
	turnThreshold)

//-----------------------------------------------

struct SimulationConfig
{
	float timeStep{ 0.f };
	float hitRadius{ 0.f };

#ifdef DebugPrint
	void print() const;
#endif
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SimulationConfig,
	timeStep,
	hitRadius)

//-----------------------------------------------

struct Config
{
	static Config createFromJSONFile(const std::string& inFileName);

	void writeToJSONFile(const std::string& inFileName) const;

#ifdef DebugPrint
	void print() const;
#endif

	DroneConfig drone{ };

	std::string ammo{ };
	SimulationConfig simulation{ };

	float targetArrayTimeStep{ 0.f };
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Config,
	drone,
	ammo,
	simulation,
	targetArrayTimeStep)

// ---

enum EDroneState
{
	STOPPED,
	ACCELERATING,
	DECELERATING,
	TURNING_PLUS,
	TURNING_MINUS,
	MOVING
};

const char* toString(const EDroneState inDroneState);

// ---

struct DroneState
{
	DroneState() = default;
	DroneState(const DroneConfig& inConfig);

	Coord position{ };
	float direction{ 0.f };

	float velocity{ 0.f };

	EDroneState state{ STOPPED };
	float targetAngle{ 0.f };

	int targetIndex{ 0 };
	Coord dropPoint{ };
	Coord aimPoint{ };
	Coord predictedTarget{ };

	float acceleration{ 0.f };
	float angularSpeed{ 0.f };
};

// -----------------------------------------------

class AmmoConfig
{
public:
	static AmmoConfig createFromJSONFile(const std::string& inFileName);

	const AmmoParams* getParams(const std::string& inParamsName) const;

#ifdef DebugPrint
	void print() const;
#endif

private:
	std::map<std::string, AmmoParams> paramsMap{ };
};

// -----------------------------------------------

struct TargetState
{
	Coord position;
};
