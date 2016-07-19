cmake_minimum_required(VERSION 2.8)
project(proctrl)

enable_testing()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

FIND_PACKAGE( Boost 1.57 COMPONENTS system serialization signals REQUIRED )
INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )

include_directories (${PROJECT_SOURCE_DIR}/include)
add_subdirectory (src)
add_subdirectory (test)
add_subdirectory (sample)