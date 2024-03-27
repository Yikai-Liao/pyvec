cmake_minimum_required(VERSION 3.20)
project(pyvec)

set(CMAKE_CXX_STANDARD 17)

Include(FetchContent)
FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG        v3.4.0 # or a later release
)

FetchContent_MakeAvailable(Catch2)
list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)

add_executable(pyvec_test test.cpp pyvec.hpp pyvec2.hpp)
add_executable(test2 test2.cpp pyvec2.hpp)
target_link_libraries(pyvec_test PRIVATE Catch2::Catch2WithMain)

include(CTest)
include(Catch)
catch_discover_tests(pyvec_test)