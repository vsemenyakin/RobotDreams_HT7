#include "config/FileConfigLoader.hpp"

// ---------------

class FileConfigLoader : public IConfigLoader {
public:
	FileConfigLoader(const char* inAmmoConfigFileName, const char* inConfigFileName);

	const AmmoConfig& getAmmoConfig() const override;
	const Config& getConfig() const override;

private:
	AmmoConfig ammoConfig;
	Config config;
};

// --------------

FileConfigLoader::FileConfigLoader(const char* inAmmoConfigFileName, const char* inConfigFileName) :
	ammoConfig(AmmoConfig::createFromJSONFile(inAmmoConfigFileName)),
	config(Config::createFromJSONFile(inConfigFileName))
{
}

const AmmoConfig& FileConfigLoader::getAmmoConfig() const {
	return ammoConfig;
}

const Config& FileConfigLoader::getConfig() const {
	return config;
}

// --------------

std::unique_ptr<IConfigLoader> createFileConfigLoader(const char* inAmmoConfigFileName, const char* inConfigFileName) {
	return std::make_unique<FileConfigLoader>(inAmmoConfigFileName, inConfigFileName);
}
