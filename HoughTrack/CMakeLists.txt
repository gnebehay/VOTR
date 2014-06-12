# -------------------------------------------------------------------- #
# Author: Martin Godec
#         godec@icg.tugraz.at
# -------------------------------------------------------------------- #

cmake_minimum_required (VERSION 2.6)
project (HoughTracker)
	SET(HoughTracker_VERSION_MAJOR 1)
	SET(HoughTracker_VERSION_MINOR 0)

	MESSAGE(STATUS "--------------------------------------------------")
	MESSAGE(STATUS "Project: HoughTracker v${HoughTracker_VERSION_MAJOR}.${HoughTracker_VERSION_MINOR}")
	MESSAGE(STATUS "by Martin Godec <godec@icg.tugraz.at>")
	MESSAGE(STATUS "--------------------------------------------------")

# -------------------------------------------------------------------- #
# Change to configure for your system / preferences
# -------------------------------------------------------------------- #

	FIND_PACKAGE( OpenCV REQUIRED )


    if(NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
    endif()

	SET( BOOST_LIB /usr/lib ) 							# set Boost library dir (default: /usr/lib)
	SET( BOOST_INCLUDE /usr/include/boost ) 			# set Boost include dir (default: /usr/include)

# -------------------------------------------------------------------- #
# Application setup
# -------------------------------------------------------------------- #

	SET ( APPLICATION_NAME Track )
	SET ( APPLICATION_SOURCES code/main.cpp code/fern.cpp code/features.cpp code/capture.cpp code/parameters.cpp code/utilities.cpp)

# -------------------------------------------------------------------- #
# Change with caution!
# -------------------------------------------------------------------- #

	MESSAGE(STATUS "Building -- ${CMAKE_BUILD_TYPE} -- Version")
	
	SET(CMAKE_VERBOSE_MAKEFILE ON)
	SET(OPTIMIZATION_FLAG_DEBUG "-O0 -pg")
	SET(OPTIMIZATION_FLAG_RELEASE "-O3 -march=native -mtune=native -msse3 -mfpmath=sse -fomit-frame-pointer -finline-functions -floop-optimize -ffinite-math-only")
	SET(WARNING_FLAG_DEBUG "-Wall -Winit-self -Wsequence-point -Wunused")
	SET(WARNING_FLAG_RELEASE "-Wall -Winit-self -Wsequence-point -Wunused -Wno-deprecated")

	IF( CMAKE_BUILD_TYPE MATCHES Release )
		SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OPTIMIZATION_FLAG_RELEASE} -fmessage-length=0 -pipe ${WARNING_FLAG_RELEASE}")
	ELSE( CMAKE_BUILD_TYPE MATCHES Release )
		SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OPTIMIZATION_FLAG_DEBUG} -fmessage-length=0 -pipe ${WARNING_FLAG_DEBUG}")
	ENDIF( CMAKE_BUILD_TYPE MATCHES Release )

	LINK_DIRECTORIES(
		${OPENCV_LIB}
		${BOOST_LIB}
	)

	INCLUDE_DIRECTORIES(
		${OPENCV_INCLUDE}
		${BOOST_INCLUDE}
	)

	ADD_EXECUTABLE (
		${APPLICATION_NAME}
		${APPLICATION_SOURCES}
	)

	TARGET_LINK_LIBRARIES (
		${APPLICATION_NAME}
		${OpenCV_LIBS}
		config++
		boost_filesystem 		# Lookup the correct library name if this does not work!
		boost_system
	)

