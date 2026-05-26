#pragma once

#include "config/FileConfigLoader.hpp"

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
