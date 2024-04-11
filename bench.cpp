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
    const size_t num = 10000;

    std::vector v(num, 1);
    pyvec<int>  pv(v.begin(), v.end());
    std::vector sv(num, std::make_shared<int>(1));

    nanobench::Bench()
        .minEpochIterations(3000)
        .run(
            "vector::push_back",
            [&]() {
                std::vector<int> v;
                v.reserve(num);
                for (int i = 0; i < num; ++i) { v.push_back(i); }
                nanobench::doNotOptimizeAway(v);
            }
        )
        .run(
            "pyvec ::push_back",
            [&]() {
                pyvec<int> v{};
                v.reserve(num);
                for (int i = 0; i < num; ++i) { v.push_back(i); }
                nanobench::doNotOptimizeAway(v);
            }
        )
        // deepcopy
        .run(
            "vector::deepcopy",
            [&]() {
                std::vector<int> v2(v.begin(), v.end());
                nanobench::doNotOptimizeAway(v2);
            }
        )
        .run("pyvec ::shallowcopy", [&]() { nanobench::doNotOptimizeAway(pv.copy()); })
        .run("pyvec ::deepcopy", [&]() { nanobench::doNotOptimizeAway(pv.deepcopy()); })
        .run("pyvec ::collect", [&]() { nanobench::doNotOptimizeAway(pv.collect()); })
        .run("pyvec ::from_vec", [&]() { nanobench::doNotOptimizeAway(pyvec<int>{v}); })
        .run(
            "vector::sort",
            [&]() {
                std::sort(v.begin(), v.end(), [](int a, int b) { return a > b; });
                nanobench::doNotOptimizeAway(v);
            }
        )
        .run(
            "pyvec ::sort",
            [&]() {
                pv.sort(true);
                nanobench::doNotOptimizeAway(pv);
            }
        )
        .run(
            "vector::filter",
            [&]() {
                std::vector<int> v2;
                std::copy_if(v.begin(), v.end(), std::back_inserter(v2), [](int i) { return i % 2 == 0; });
                nanobench::doNotOptimizeAway(v2);
            }
        )
        .run(
            "pyvec ::filter",
            [&]() {
                auto pv2 = pv.copy();
                pv2.filter([](int i) { return i % 2 == 0; });
                nanobench::doNotOptimizeAway(pv2);
            }
        )
        // clang-format off
    ;
    // clang-format on
}