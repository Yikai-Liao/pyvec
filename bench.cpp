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
    nanobench::Bench()
        .minEpochIterations(1000)
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
            "pyvec::push_back",
            [&]() {
                pyvec<int> v;
                v.reserve(num);
                for (int i = 0; i < num; ++i) { v.push_back(i); }
                nanobench::doNotOptimizeAway(v);
            }
        )
        .run(
            "shared_vec::push_back",
            [&]() {
                std::vector<std::shared_ptr<int>> v;
                v.reserve(num);

                auto capsule = std::make_shared<std::vector<int>>();
                capsule->reserve(num);

                for (int i = 0; i < num; ++i) {
                    capsule->push_back(i);
                    v.emplace_back(capsule, &capsule->back());
                }
                nanobench::doNotOptimizeAway(v);
            }
        )

        // clang-format off
    ;
    // clang-format on
}