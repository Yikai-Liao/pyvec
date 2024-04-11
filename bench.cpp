//
// Created by lyk on 24-4-11.
//
#include <iostream>
#include <memory>
#include <vector>
#include <nanobench.h>

#include "pyvec.hpp"

using namespace ankerl;

int main() {
    std::cout << std::endl << "vector" << std::endl;
    nanobench::Bench().minEpochIterations(10000).run("push_back", [&]() {
        std::vector<int> v;
        v.reserve(10000);
        for (int i = 0; i < 1000; ++i) { v.push_back(i); }
        nanobench::doNotOptimizeAway(v);
    });

    std::cout << std::endl << "pyvec" << std::endl;

    nanobench::Bench().minEpochIterations(10000).run("push_back", [&]() {
        pyvec<int> v;
        v.reserve(10000);
        for (int i = 0; i < 1000; ++i) { v.push_back(i); }
        nanobench::doNotOptimizeAway(v);
    });
}