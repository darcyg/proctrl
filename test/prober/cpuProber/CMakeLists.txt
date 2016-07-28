cmake_minimum_required(VERSION 2.8)

project(proctrlTest)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_GNU_SOURCE")

enable_testing()

file(GLOB cpuprober_SRC
        "*.h"
        "*.cpp"
        "cpulimit.c"
        "list.c"
        "memrchr.c"
        "process_group.c"
        "process_iterator.c"
        )

add_executable(cpuProberTest ${cpuprober_SRC})
target_link_libraries(cpuProberTest gtest cpu_prober proctrl)

add_test(CpuProberTest cpuProberTest)

