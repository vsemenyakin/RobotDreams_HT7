#include "config/ConfigLoaderFactory.hpp"
#include "config/FileConfigLoader.hpp"

#include <assert.h>

std::unique_ptr<IConfigLoader> createLoader(LoaderType type,
    const std::any& firstArg,
    const std::any& secondArg)
{
	switch(type) {
		case LoaderType::FILE:
        {
            const auto ammoConfigFileName = std::any_cast<std::string>(firstArg);
            const auto configFileName = std::any_cast<std::string>(secondArg);
            return createFileConfigLoader(ammoConfigFileName, configFileName);
        }
		
		default:
			assert(false && "Unknown loader type");
			return nullptr;
	}
}
