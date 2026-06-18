#include "MissionProcessor.hpp"

namespace fileNames
{
	constexpr char ammo[]{ "ammo.json" };
	constexpr char config[]{ "config.json" };
	constexpr char targets[]{ "targets.json" };

	constexpr char simulation[]{ "simulation.json" };
}

// -----------------------------

int main()
{
	MissionProcessor missionProcessor{
		fileNames::ammo, fileNames::config, fileNames::targets
	};

	missionProcessor.start();

	missionProcessor.writeResults(fileNames::simulation);

	return 0;
}
