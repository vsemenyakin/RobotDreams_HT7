#pragma once

#include "interfaces/IBallisticSolver.hpp"

#include <any>

enum class SolverType
{
	ANALYTICAL,
	TABLE
};

std::unique_ptr<IBallisticSolver> createSolver(SolverType type, std::any firstArg = {});
