cmake_minimum_required(VERSION 3.20)

project(ballistics CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(ballistics_calculator
	src/BallisticSolver.cpp
	include/BallisticSolver.hpp
	src/CommonTypes.cpp
	include/CommonTypes.hpp
	src/ConfigLoader.cpp
	include/ConfigLoader.hpp
	include/json.hpp
	src/main.cpp
	src/SimulationStorage.cpp
	include/SimulationStorage.hpp
	src/TargetProvider.cpp
	include/TargetProvider.hpp
	include/Utils.hpp
	include/MissionProcessor.hpp
	src/MissionProcessor.cpp)

target_include_directories(ballistics_calculator PUBLIC include)
