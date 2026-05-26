cmake_minimum_required(VERSION 3.20)

project(ballistics CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(ballistics_calculator
	src/solvers/BallisticSolver.cpp
	include/solvers/BallisticSolver.hpp
	src/CommonTypes.cpp
	include/CommonTypes.hpp
	src/config/ConfigLoader.cpp
	include/config/ConfigLoader.hpp
	include/json.hpp
	src/main.cpp
	src/SimulationStorage.cpp
	include/SimulationStorage.hpp
	src/providers/TargetProvider.cpp
	include/providers/TargetProvider.hpp
	include/Utils.hpp
	include/MissionProcessor.hpp
	src/MissionProcessor.cpp
	include/interfaces/IBallisticSolver.hpp
	include/interfaces/IConfigLoader.hpp
	include/interfaces/ITargetProvider.hpp)

target_include_directories(ballistics_calculator PUBLIC include)
