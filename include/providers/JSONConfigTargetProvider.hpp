#pragma once

#include "interfaces/ITargetProvider.hpp"

#include <memory>
#include <string>

std::unique_ptr<ITargetProvider> createJSONTargetProvider(
    const std::string& inConfigFileName, const float inTargetArrayTimeStep);
