#pragma once

#include "interfaces/ITargetProvider.hpp"

#include <any>

enum class ProviderType
{
	JSON
};

std::unique_ptr<ITargetProvider> createProvider(ProviderType type,
	const std::any& firstArg = {},
	const std::any& secondArg = {});
