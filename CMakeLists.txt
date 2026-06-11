cmake_minimum_required(VERSION 3.20)

project(ballistics CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(ballistics_calculator
	src/solvers/SimpleBallisticSolver.cpp
	include/solvers/SimpleBallisticSolver.hpp
	include/solvers/BallisticSolverFabric.hpp
	src/solvers/BallisticSolverFabric.cpp
	src/CommonTypes.cpp
	include/CommonTypes.hpp
	src/config/FileConfigLoader.cpp
	include/config/FileConfigLoader.hpp
	include/config/ConfigLoaderFabric.hpp
	src/config/ConfigLoaderFabric.cpp
	include/json.hpp
	src/main.cpp
	src/SimulationStorage.cpp
	include/SimulationStorage.hpp
	src/providers/JSONConfigTargetProvider.cpp
	include/providers/JSONConfigTargetProvider.hpp
	include/providers/TargetProviderFabric.hpp
	src/providers/TargetProviderFabric.cpp
	include/Utils.hpp
	include/MissionProcessor.hpp
	src/MissionProcessor.cpp
	include/interfaces/IBallisticSolver.hpp
	include/interfaces/IConfigLoader.hpp
	include/interfaces/ITargetProvider.hpp
	include/solvers/TableBallisticSolver.hpp
	src/solvers/TableBallisticSolver.cpp)

target_include_directories(ballistics_calculator PUBLIC include)
