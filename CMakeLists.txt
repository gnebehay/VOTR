project(CompressiveTracking)

cmake_minimum_required(VERSION 2.6)

find_package(OpenCV REQUIRED)

set(CMAKE_BUILD_TYPE Release)

INCLUDE_DIRECTORIES (${OpenCV_INCLUDE_DIRS})

add_executable(ct
	CompressiveTracker.cpp
	RunTracker.cpp
	)

target_link_libraries(ct ${OpenCV_LIBS})
