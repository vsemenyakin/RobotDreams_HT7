#pragma once

#include "interfaces/ITargetProvider.hpp"

#include <memory>

std::unique_ptr<ITargetProvider> createJSONTargetProvider(const char* inConfigFileName, const float inTargetArrayTimeStep);
