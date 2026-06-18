#include "providers/TargetProviderFactory.hpp"
#include "providers/JSONConfigTargetProvider.hpp"

std::unique_ptr<ITargetProvider> createProvider(ProviderType type,
	const std::any& firstArg,
	const std::any& secondArg,
	const std::any& thirdArg)
{
	switch(type) {
		case ProviderType::JSON:
		{
			const auto configFileName = std::any_cast<std::string>(firstArg);
			const auto targetArrayTimeStep = std::any_cast<float>(secondArg);
			const auto tickDeltaTime = std::any_cast<float>(thirdArg);
			return createJSONTargetProvider(configFileName, targetArrayTimeStep, tickDeltaTime);
		}
		
		default:
			assert(false && "Unknown provider type");
			return nullptr;
	}
}
