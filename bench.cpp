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
        // deepcopy
        .run(
            "vector::deepcopy",
            [&]() {
                std::vector<int> v2(v.begin(), v.end());
                nanobench::doNotOptimizeAway(v2);
            }
        )
        .run(
            "pyvec::deepcopy",
            [&]() {
                pyvec<int> v2(pv.begin(), pv.end());
                nanobench::doNotOptimizeAway(v2);
            }
        )
        .run(
            "shared_vec::deepcopy",
            [&]() {
                auto capsule = std::make_shared<std::vector<int>>();
                capsule->reserve(num);
                auto sv2 = std::vector<std::shared_ptr<int>>();
                sv2.reserve(num);
                for(auto& i : v) {
                    capsule->push_back(i);
                    sv2.emplace_back(capsule, &capsule->back());
                }
                nanobench::doNotOptimizeAway(sv2);
            }
        )


        // clang-format off
    ;
    // clang-format on
}