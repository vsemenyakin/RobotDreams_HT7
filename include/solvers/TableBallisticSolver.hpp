#pragma once

#include "interfaces/IBallisticSolver.hpp"

#include <memory>

std::unique_ptr<IBallisticSolver> createTableBallisticSolver(const std::string& tableFileName);
