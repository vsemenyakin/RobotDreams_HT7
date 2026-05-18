#pragma once

#include "json.hpp"

using json = nlohmann::json;

template<typename Type>
void readArrayFromJSON(Type*& outArray, size_t& outArraySize, const json& inJSON)
{
	outArraySize = inJSON.size();
	if (outArraySize == 0)
	{
		outArray = nullptr;
		outArraySize = 0;
		return;
	}

	outArray = new Type[outArraySize];
	for (size_t index = 0; index < outArraySize; ++index)
	{
		outArray[index] = inJSON[index];
	}
}
