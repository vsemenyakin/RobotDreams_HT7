#pragma once

#include "solvers/SimpleBallisticSolver.hpp"

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



