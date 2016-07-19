cmake_minimum_required(VERSION 2.8)

set(REPO ~/ClionProjects/.repo)

project(proctrlTest)

enable_testing()

add_subdirectory(googletest)

set(test_SRCS processTest.cpp)
add_executable(controlTest ${test_SRCS})
target_link_libraries(controlTest gtest proctrl)

add_test(ControlTest controlTest)