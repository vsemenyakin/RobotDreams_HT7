#include "solvers/BallisticSolverFabric.hpp"
#include "solvers/SimpleBallisticSolver.hpp"
#include "solvers/TableBallisticSolver.hpp"

std::unique_ptr<IBallisticSolver> createSolver(SolverType type, std::any firstArg) {
	switch(type) {
		case SolverType::ANALYTICAL:
			return createSimpleBallisticSolver();
		
		case SolverType::TABLE:
		{
			const auto tableFileName = std::any_cast<std::string>(firstArg);
			return createTableBallisticSolver(tableFileName);
		}

		default:
			assert(false && "Unknown solver type");
			return nullptr;
	}
}
