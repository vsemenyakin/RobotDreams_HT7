#pragma once

#include "CommonTypes.hpp"

class IBallisticSolver
{
public:
	virtual void computeAmmoDrop(
		float& outFlightTime, float& outHorizontalDistance,
		const float attackSpeed,
		const float ammo_zPos,
		const AmmoParams& ammoParams) = 0;

	virtual ~IBallisticSolver() = default;
};
