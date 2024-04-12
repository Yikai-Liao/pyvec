//
// Created by lyk on 24-4-11.
//
#include <iostream>
#include <memory>
#include <vector>
#include <nanobench.h>

#include "pyvec.hpp"

using namespace ankerl;
using namespace pycontainer;

int main() {
    const size_t num = 10000;

    std::vector v(num, 1);
    pyvec<int>  pv(v.begin(), v.end());
    std::vector sv(num, std::make_shared<int>(1));

    nanobench::Bench()
        .minEpochIterations(3000)
        .run(
            "vector::push_back",
            [num]() {
                std::vector<int> v;
                v.reserve(num);
                for (int i = 0; i < num; ++i) { v.push_back(i); }
                nanobench::doNotOptimizeAway(v);
            }
        )
        .run(
            "pyvec ::push_back",
            [num]() {
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
                std::vector v2(v);
                std::sort(v2.begin(), v2.end(), [](int a, int b) { return a > b; });
                nanobench::doNotOptimizeAway(v2);
            }
        )
        .run(
            "pyvec ::sort",
            [&]() {
                pyvec<int> v2 = pv.copy();
                v2.sort(true);
                nanobench::doNotOptimizeAway(v2);
            }
        )
        .run(
            "vector::filter",
            [&]() {
                std::vector<int> v2;
                std::copy_if(v.begin(), v.end(), std::back_inserter(v2), [](int i) {
                    return i % 2 == 0;
                });
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
        .run(
            "vector::sum",
            [&]() {
                size_t sum = 0;
                for (const auto& i : v) { sum = (sum + i) % 1000000007; }
                nanobench::doNotOptimizeAway(sum);
            }
        )
        .run(
            "pyvec ::sum",
            [&]() {
                size_t sum = 0;
                for (const auto& i : pv) { sum = (sum + i) % 1000000007; }
                nanobench::doNotOptimizeAway(sum);
            }
        )
        // clang-format off
    ;
    // clang-format on
}