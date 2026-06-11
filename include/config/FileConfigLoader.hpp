#pragma once

#include <memory>
#include <string>

class IConfigLoader;

std::unique_ptr<IConfigLoader> createFileConfigLoader(
    const std::string& inAmmoConfigFileName,
    const std::string& inConfigFileName);
