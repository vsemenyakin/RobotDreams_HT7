#pragma once

#include "CommonTypes.hpp"

class IConfigLoader {
public:
	virtual const AmmoConfig& getAmmoConfig() const = 0;
	virtual const Config& getConfig() const = 0;

	virtual ~IConfigLoader() = default;
};
