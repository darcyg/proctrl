cmake_minimum_required (VERSION 2.8)

SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

add_library (proctrl SHARED process.cpp)

target_link_libraries (proctrl ${Boost_LIBRARIES})