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

// ---------------

IBallisticSolver* createSimpleBallisticSolver();

// ---------------

enum class SolverType
{
	ANALYTICAL
};
 
template<typename ... Args>
IBallisticSolver* createSolver(SolverType type, Args&& ... args) {
	switch(type) {
		case SolverType::ANALYTICAL:
			return createSimpleBallisticSolver();
		
		default:
			assert(false && "Unknown solver type");
			return nullptr;
	}
}



