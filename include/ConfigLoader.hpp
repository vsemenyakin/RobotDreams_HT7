#pragma once

#include "CommonTypes.hpp"

class IConfigLoader {
public:
	virtual const AmmoConfig& getAmmoConfig() const = 0;
	virtual const Config& getConfig() const = 0;
};

// ---------------

IConfigLoader* createFileConfigLoader(const char* inAmmoConfigFileName, const char* inConfigFileName);

// ---------------

enum class LoaderType
{
	FILE
};

template<typename ... Args>
IConfigLoader* createLoader(LoaderType type, Args&& ... args) {
	switch(type) {
		case LoaderType::FILE:
			return createFileConfigLoader(std::forward<Args>(args)...);
		
		default:
			assert(false && "Unknown loader type");
			return nullptr;
	}
}
