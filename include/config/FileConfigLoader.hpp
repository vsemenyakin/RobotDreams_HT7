#pragma once

#include "interfaces/IConfigLoader.hpp"

#include <memory>

std::unique_ptr<IConfigLoader> createFileConfigLoader(const char* inAmmoConfigFileName, const char* inConfigFileName);
