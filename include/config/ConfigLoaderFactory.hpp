#pragma once

#include "interfaces/IConfigLoader.hpp"

#include <any>
#include <memory>

enum class LoaderType
{
	FILE
};

std::unique_ptr<IConfigLoader> createLoader(LoaderType type,
	const std::any& firstArg = {},
	const std::any& secondArg = {});
