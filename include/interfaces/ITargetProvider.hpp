#pragma once

#include "CommonTypes.hpp"

class ITargetProvider {
public:
    virtual size_t getTargetCount() const = 0;
    virtual TargetState getTarget(const int index) const = 0;

    virtual bool isThreadReady() const = 0;
    virtual void start() = 0;
    virtual void stop() = 0;

	virtual ~ITargetProvider() = default;
};
