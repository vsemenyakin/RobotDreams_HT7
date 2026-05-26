#pragma once

#include "interfaces/ITargetProvider.hpp"

ITargetProvider* createJSONTargetProvider(const char* inConfigFileName, const float inTargetArrayTimeStep);

// ---------------

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
