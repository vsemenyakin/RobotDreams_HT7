#pragma once

#include "interfaces/IConfigLoader.hpp"

IConfigLoader* createFileConfigLoader(const char* inAmmoConfigFileName, const char* inConfigFileName);
