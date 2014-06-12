project(LearnMatch)

cmake_minimum_required(VERSION 2.6)

find_package(OpenCV REQUIRED)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

INCLUDE_DIRECTORIES (src/3rdparty/ /usr/include/eigen3 ${OpenCV_INCLUDE_DIRS})

add_executable(learnmatch
    src/main.cpp
    src/Config.cpp
    src/Detector.cpp
    src/Model.cpp
    src/RobustHomography.cpp
    src/Stats.cpp
    src/Video.cpp
    src/Utils.cpp
    src/3rdparty/GraphUtils/GraphUtils.cpp)

set(CMAKE_CXX_FLAGS "-fpermissive")

target_link_libraries(learnmatch ${OpenCV_LIBS})
