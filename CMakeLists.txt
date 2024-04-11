cmake_minimum_required(VERSION 3.20)
project(pyvec)

set(CMAKE_CXX_STANDARD 17)

Include(FetchContent)
FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG        v3.4.0 # or a later release
)

FetchContent_Declare(
        nanobench
        GIT_REPOSITORY https://github.com/martinus/nanobench.git
        GIT_TAG        v4.3.11 # or a later release
)

FetchContent_MakeAvailable(Catch2 nanobench)
list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)

add_executable(pyvec_test test.cpp pyvec.hpp)
target_link_libraries(pyvec_test PRIVATE Catch2::Catch2WithMain)

add_executable(bench bench.cpp pyvec.hpp)
target_link_libraries(bench PRIVATE nanobench)

include(CTest)
include(Catch)
catch_discover_tests(pyvec_test)