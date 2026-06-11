#pragma once

#include "CommonTypes.hpp"

class ITargetProvider {
public:
    virtual size_t getTargetCount() = 0;
    virtual TargetState getTarget(const int index, const float inSimulationTime) = 0;

	virtual ~ITargetProvider() = default;
};
