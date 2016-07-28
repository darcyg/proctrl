cmake_minimum_required(VERSION 2.8)

project(proctrlTest)

enable_testing()

set(test_SRCS controlTest.cpp)
add_executable(controlTest ${test_SRCS})
target_link_libraries(controlTest gtest proctrl)

add_executable(redirectIOTest redirectIOTest.cpp)
target_link_libraries(redirectIOTest gtest proctrl)

add_test(ControlTest controlTest)
add_test(RedirectIOTest redirectIOTest)
