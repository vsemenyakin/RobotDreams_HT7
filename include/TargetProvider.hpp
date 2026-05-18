#pragma once

#include "CommonTypes.hpp"

struct TargetState
{
	Coord position;
};

// -------------

class ITargetProvider {
public:
    virtual size_t getTargetCount() = 0;
    virtual TargetState getTarget(const int index, const float inSimulationTime) = 0;

	virtual ~ITargetProvider() = default;
};

// -------------

ITargetProvider* createJSONTargetProvider(const char* inConfigFileName, const float inTargetArrayTimeStep);

// -------------

enum class ProviderType
{
	JSON
};

template<typename ... Args>
ITargetProvider* createProvider(ProviderType type, Args&& ... args) {
	switch(type) {
		case ProviderType::JSON:
			return createJSONTargetProvider(std::forward<Args>(args)...);
		
		default:
			assert(false && "Unknown provider type");
			return nullptr;
	}
}
