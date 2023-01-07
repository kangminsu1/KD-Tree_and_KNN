cmake_minimum_required(VERSION 2.8)

find_package(OpenCV REQUIRED)
find_package(PCL REQUIRED)
include_directories(
	${OpenCV_INCLUDE_DIRS}
	${PCL_INCLUDE_DIRS}
)

add_definitions(${PCL_DEFINITIONS})
link_directories(${PCL_LIBRARY_DIRS})

if (CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS "-std=c++14 -Wall -O3")
endif (CMAKE_COMPILER_IS_GNUCXX)

file(GLOB srcs main.cpp header_kdtree.h)

add_executable(kdtree ${srcs})
target_link_libraries(kdtree ${OpenCV_LIBS} ${PCL_LIBRARIES})