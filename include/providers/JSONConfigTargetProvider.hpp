#pragma once

#include "interfaces/ITargetProvider.hpp"

ITargetProvider* createJSONTargetProvider(const char* inConfigFileName, const float inTargetArrayTimeStep);
