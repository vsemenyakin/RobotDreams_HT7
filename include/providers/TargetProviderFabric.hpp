#pragma once

#include "providers/JSONConfigTargetProvider.hpp"

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
