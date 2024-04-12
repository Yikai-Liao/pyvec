cmake_minimum_required(VERSION 3.20)
project(pyvec)

option(BUILD_PYVEC_TESTS "Build tests" OFF)
option(BUILD_PYVEC_BENCHMARKS "Build benchmarks" OFF)

add_library(pyvec INTERFACE)
add_library(pyvec::pyvec ALIAS pyvec)
target_compile_features(pyvec INTERFACE cxx_std_20)
target_include_directories(pyvec INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

if (BUILD_PYVEC_TESTS)
    Include(FetchContent)
    FetchContent_Declare(
            Catch2
            GIT_REPOSITORY https://github.com/catchorg/Catch2.git
            GIT_TAG v3.4.0 # or a later release
    )
    FetchContent_MakeAvailable(Catch2)
    list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
    add_executable(pyvec_test test.cpp)
    target_link_libraries(pyvec_test PRIVATE Catch2::Catch2WithMain pyvec)
    include(CTest)
    include(Catch)
    catch_discover_tests(pyvec_test)
endif ()

if (BUILD_PYVEC_BENCHMARKS)
    Include(FetchContent)
    FetchContent_Declare(
            nanobench
            GIT_REPOSITORY https://github.com/martinus/nanobench.git
            GIT_TAG v4.3.11 # or a later release
    )
    FetchContent_MakeAvailable(nanobench)
    add_executable(bench bench.cpp)
    target_link_libraries(bench PRIVATE nanobench pyvec)
endif ()
