project(FragTrack)

cmake_minimum_required(VERSION 2.6)

find_package(OpenCV REQUIRED)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

INCLUDE_DIRECTORIES ( ${OpenCV_INCLUDE_DIRS})

add_executable(FragTrack
	fragtrack_envelope.cpp
	Fragments_Tracker.cpp
	emd.cpp
    )

target_link_libraries(FragTrack ${OpenCV_LIBS})
