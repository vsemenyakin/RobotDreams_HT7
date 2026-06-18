#include "config/FileConfigLoader.hpp"
#include "interfaces/IConfigLoader.hpp"

// ---------------

class FileConfigLoader : public IConfigLoader {
public:
	FileConfigLoader(const std::string& inAmmoConfigFileName, const std::string& inConfigFileName);

	const AmmoConfig& getAmmoConfig() const override;
	const Config& getConfig() const override;

private:
	AmmoConfig ammoConfig;
	Config config;
};

// --------------

FileConfigLoader::FileConfigLoader(const std::string& inAmmoConfigFileName, const std::string& inConfigFileName) :
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

std::unique_ptr<IConfigLoader> createFileConfigLoader(
	const std::string& inAmmoConfigFileName,
	const std::string& inConfigFileName)
{
	return std::make_unique<FileConfigLoader>(inAmmoConfigFileName, inConfigFileName);
}
