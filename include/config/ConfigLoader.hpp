#pragma once

#include "interfaces/IConfigLoader.hpp"

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
