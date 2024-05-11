//
// Created by lyk on 24-3-9.
//
#include "pyvec.hpp"
#include <list>
#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <algorithm>

using std::nullopt;
using namespace pycontainer;

TEST_CASE("pyvec basic editing", "[pyvec]") {
    pyvec<int> v{1, 2, 3, 4, 5};
    REQUIRE(v.size() == 5);
    REQUIRE(v.capacity() >= 5);
    REQUIRE(v.collect() == std::vector<int>{1, 2, 3, 4, 5});

    SECTION("constructors") {
        std::vector<int> tmp_vec{1, 2, 3, 4, 5};
        pyvec<int>       tmp1(tmp_vec.begin(), tmp_vec.end());
        REQUIRE(tmp1.collect() == tmp_vec);
        pyvec<int> tmp2(tmp1);
        REQUIRE(tmp2.collect() == tmp_vec);
        pyvec<int> tmp3(std::move(tmp2));
        REQUIRE(tmp3.collect() == tmp_vec);
        std::list<int> tmp_list{1, 2, 3, 4, 5};
        pyvec<int>     tmp4(tmp_list.begin(), tmp_list.end());
        REQUIRE(tmp4.collect() == tmp_vec);
        pyvec<int> tmp5(tmp4.begin(), tmp4.end());
        REQUIRE(tmp5.collect() == tmp_vec);

        pyvec<int> tmp6{std::move(tmp5)};
        REQUIRE(tmp6.collect() == tmp_vec);

        std::vector<int> tmp_vec2{tmp_vec};
        pyvec<int>       tmp8(std::move(tmp_vec2));
        REQUIRE(tmp8.collect() == tmp_vec);

        // rbegin, rend
        pyvec<int> tmp9(tmp8.rbegin(), tmp8.rend());
        REQUIRE(tmp9.collect() == std::vector<int>{5, 4, 3, 2, 1});

        pyvec<int> tmp10{};
        REQUIRE(tmp10.collect().empty());
    }

    SECTION("push_back and emplace_back") {
        v.push_back(6);
        REQUIRE(v.size() == 6);
        REQUIRE(v.back() == 6);

        v.emplace_back(7);
        REQUIRE(v.size() == 7);
        REQUIRE(v.back() == 7);

        pyvec<int> tmp{};
        tmp.push_back(1);
        tmp.emplace_back(2);
        REQUIRE(tmp.collect() == std::vector<int>{1, 2});
    }

    SECTION("insert and emplace") {
        auto iter = v.emplace(v.begin() + 2, 6);
        REQUIRE(*iter == 6);
        REQUIRE(v.collect() == std::vector<int>{1, 2, 6, 3, 4, 5});
        iter = v.insert(v.begin() + 3, 7);
        REQUIRE(*iter == 7);
        REQUIRE(v.collect() == std::vector<int>{1, 2, 6, 7, 3, 4, 5});
        iter = v.insert(v.begin() + 4, 3, 8);
        REQUIRE(*iter == 8);
        REQUIRE(v.collect() == std::vector<int>{1, 2, 6, 7, 8, 8, 8, 3, 4, 5});
        iter = v.insert(v.begin() + 5, {9, 10, 11});
        REQUIRE(*iter == 9);
        REQUIRE(v.collect() == std::vector<int>{1, 2, 6, 7, 8, 9, 10, 11, 8, 8, 3, 4, 5});
        {
            std::vector<int> tmp{12, 13, 14};
            iter = v.insert(v.begin() + 6, tmp.begin(), tmp.end());
            REQUIRE(*iter == 12);
            REQUIRE(
                v.collect() == std::vector<int>{1, 2, 6, 7, 8, 9, 12, 13, 14, 10, 11, 8, 8, 3, 4, 5}
            );
        }
        {
            std::list<int> tmp{15, 16, 17};
            iter = v.insert(v.begin() + 7, tmp.begin(), tmp.end());
            REQUIRE(*iter == 15);
            REQUIRE(
                v.collect() ==
                std::vector<int>{1, 2, 6, 7, 8, 9, 12, 15, 16, 17, 13, 14, 10, 11, 8, 8, 3, 4, 5}
            );
        }
    }

    SECTION("swap") {
        pyvec<int> tmp{1, 2, 3, -1};
        v.swap(tmp);
        REQUIRE(v.collect() == std::vector<int>{1, 2, 3, -1});
        REQUIRE(tmp.collect() == std::vector<int>{1, 2, 3, 4, 5});
    }

    SECTION("erase and pop_back") {
        auto it = v.erase(v.begin() + 2);
        REQUIRE(*it == 4);
        REQUIRE(v.collect() == std::vector<int>{1, 2, 4, 5});
        it = v.erase(v.begin() + 1, v.begin() + 3);
        REQUIRE(*it == 5);
        REQUIRE(v.collect() == std::vector<int>{1, 5});
        v.pop_back();
        REQUIRE(v.collect() == std::vector<int>{1});
    }

    SECTION("assign") {
        v.assign(3, 6);
        REQUIRE(v.collect() == std::vector<int>{6, 6, 6});
        v.assign({1, 2, 3, 4, 5});
        REQUIRE(v.collect() == std::vector<int>{1, 2, 3, 4, 5});
        pyvec<int> tmp{6, 6, 6};
        v.assign(tmp.begin(), tmp.end());
        REQUIRE(v.collect() == std::vector<int>{6, 6, 6});

        tmp.assign(5, 2);
        v = tmp;
        REQUIRE(v.collect() == std::vector<int>{2, 2, 2, 2, 2});
        tmp.assign(6, 1);
        v = std::move(tmp);
        REQUIRE(v.collect() == std::vector<int>{1, 1, 1, 1, 1, 1});
        v = {1, 2, 3, 4, 5};
        REQUIRE(v.collect() == std::vector<int>{1, 2, 3, 4, 5});

        std::vector<int> data(1000, 1);
        v.assign(data.begin(), data.end());
        REQUIRE(v.collect() == data);

        v.assign(v.begin(), v.end());
        REQUIRE(v.collect() == data);

        pyvec<int> v2;
        v2.assign(v.begin(), v.end());
        REQUIRE(v2.collect() == data);
        v2.assign(2, 2);
        REQUIRE(v2.collect() == std::vector<int>(2, 2));
    }
}

TEST_CASE("pyvec basic access", "[pyvec]") {
    pyvec<int> v{1, 2, 3, 4, 5};
    REQUIRE(v.front() == 1);
    REQUIRE(v.back() == 5);
    REQUIRE(*v.begin() == 1);
    REQUIRE(*(v.end() - 1) == 5);
    REQUIRE(*v.rbegin() == 5);
    REQUIRE(*(v.rend() - 1) == 1);
    REQUIRE(**v.pbegin() == 1);
    REQUIRE(**(v.pend() - 1) == 5);
    REQUIRE(**v.rpbegin() == 5);
    REQUIRE(**(v.rpend() - 1) == 1);
    REQUIRE(**(v.sbegin() + 2) == 3);
    REQUIRE(**(v.send() - 1) == 5);
    REQUIRE(v[2] == 3);
    REQUIRE(v.at(2) == 3);
    REQUIRE(v[2] == 3);
    REQUIRE_THROWS(v.at(5));
    REQUIRE_THROWS(v.getitem(10));
    auto d2 = v.getitem(2);
    REQUIRE(d2.use_count() == 2);
    {
        auto d3 = v.getitem(3);
        REQUIRE(d3.use_count() == 3);
        REQUIRE(d2.use_count() == 3);
    }
    REQUIRE(d2.use_count() == 2);
    {
        auto v2 = v.getitem({1, 4, 1});
        REQUIRE(d2.use_count() == 3);
    }
    REQUIRE(d2.use_count() == 2);
    {
        auto v2 = v.deepcopy();
        REQUIRE(d2.use_count() == 2);
        auto v3 = v.copy();
        REQUIRE(d2.use_count() == 3);
        auto d3 = v2.getitem(2);
        REQUIRE(d3.use_count() == 2);
        REQUIRE(d2.use_count() == 3);
        auto d4 = v3.getitem(2);
        REQUIRE(d4.use_count() == 4);
        REQUIRE(d3.use_count() == 2);
        REQUIRE(d2.use_count() == 4);
    }
    REQUIRE(d2.use_count() == 2);
    {
        pyvec<int> tmp_v{};
        v.swap(tmp_v);
        REQUIRE(d2.use_count() == 2);
        REQUIRE(d2.use_count() == 2);
    }
    REQUIRE(d2.use_count() == 1);
}

TEST_CASE("comparison operators", "[pyvec]") {
    pyvec<int> v1{1, 2, 3, 4, 5};
    pyvec<int> v2{1, 2, 3, 4, 5};
    pyvec<int> v3{1, 2, 3, 4, 6};
    pyvec<int> v4{1, 2, 3, 4};
    pyvec<int> v5{1, 2, 3, 4, 5, 6};

    REQUIRE(v1 == v2);
    // !=
    REQUIRE(v1 != v3);
    REQUIRE(v1 != v4);
    REQUIRE(v1 != v5);

    // <
    REQUIRE(v1 < v3);
    REQUIRE(v1 < v5);

    // >
    REQUIRE(v3 > v1);
    REQUIRE(v5 > v1);

    // <=
    REQUIRE(v1 <= v2);
    REQUIRE(v1 <= v1);
    REQUIRE(v1 <= v3);
    REQUIRE(v1 <= v5);

    // >=
    REQUIRE(v2 >= v1);
    REQUIRE(v1 >= v1);
    REQUIRE(v3 >= v1);
    REQUIRE(v5 >= v1);
}

TEST_CASE("memory stability", "[pyvec]") {
    pyvec<int>        v{1, 2, 3, 4, 5};
    std::vector<int*> ptrs(5);
    for (int i = 0; i < 5; ++i) { ptrs[i] = &v[i]; }
    SECTION("append & remove") {
        for (int i = 0; i < 100; ++i) {
            v.push_back(i);
            v.emplace_back(i);
        }
        for (int i = 0; i < 5; ++i) { REQUIRE(ptrs[i] == &v[i]); }
        for (int i = 0; i < 50; ++i) { v.pop_back(); }
        for (int i = 0; i < 5; ++i) { REQUIRE(ptrs[i] == &v[i]); }
        for (int i = 0; i < 20; ++i) { v.erase(v.end() - 1); }
        for (int i = 0; i < 5; ++i) { REQUIRE(ptrs[i] == &v[i]); }
    }

    SECTION("capacity") {
        v.reserve(1000);
        for (int i = 0; i < 5; ++i) {
            REQUIRE(*ptrs[i] == i + 1);
            REQUIRE(ptrs[i] == &v[i]);
        }
        v.shrink_to_fit();
        for (int i = 0; i < 5; ++i) {
            REQUIRE(*ptrs[i] == i + 1);
            REQUIRE(ptrs[i] == &v[i]);
        }
        v.resize(1000);
        for (int i = 0; i < 5; ++i) {
            REQUIRE(*ptrs[i] == i + 1);
            REQUIRE(ptrs[i] == &v[i]);
        }
        v.resize(2);
        for (int i = 0; i < 2; ++i) {
            REQUIRE(*ptrs[i] == i + 1);
            REQUIRE(ptrs[i] == &v[i]);
        }
    }

    SECTION("sort") {
        auto slice = v.getitem({0, 5, 1});

        std::sort(v.pbegin(), v.pend(), [](const int* a, const int* b) { return *a > *b; });
        REQUIRE(v.collect() == std::vector<int>{5, 4, 3, 2, 1});
        REQUIRE(
            slice.collect() == std::vector<int>{1, 2, 3, 4, 5}
        );   // sort using piter does not affect slice

        std::sort(v.pbegin(), v.pend(), [](const int* a, const int* b) { return *a < *b; });
        REQUIRE(v.collect() == std::vector<int>{1, 2, 3, 4, 5});
        REQUIRE(
            slice.collect() == std::vector<int>{1, 2, 3, 4, 5}
        );   // sort using piter does not affect slice

        std::sort(v.begin(), v.end(), [](int a, int b) { return a > b; });
        REQUIRE(v.collect() == std::vector<int>{5, 4, 3, 2, 1});
        REQUIRE(
            slice.collect() == std::vector<int>{5, 4, 3, 2, 1}
        );   // sort using iter does affect slice

        v.sort([](const auto& a) -> const auto& { return a; }, false);
        REQUIRE(v.collect() == std::vector<int>{1, 2, 3, 4, 5});
        REQUIRE(
            slice.collect() == std::vector<int>{5, 4, 3, 2, 1}
        );   // member function sort does not affect slice

        v.sort(true);
        REQUIRE(v.collect() == std::vector<int>{5, 4, 3, 2, 1});
        REQUIRE(
            slice.collect() == std::vector<int>{5, 4, 3, 2, 1}
        );   // member function sort does not affect slice
    }
}

TEST_CASE("python interface") {
    pyvec<int> v{1, 2, 3, 4, 5};

    SECTION("member functions") {
        REQUIRE(v.index(3) == 2);
        REQUIRE(v.index(3, 2) == 2);
        REQUIRE_THROWS(v.index(3, 3));
        REQUIRE_THROWS(v.index(6));
        REQUIRE(v.contains(3));
        REQUIRE(v.contains(6) == false);
        REQUIRE(v.count(3) == 1);
        REQUIRE(v.count(6) == 0);
        REQUIRE(v.is_sorted());
        REQUIRE(!v.is_sorted(true));
        REQUIRE(!v.is_sorted([](int a) { return -a; }, false));
        REQUIRE(v.is_sorted([](int a) { return -a; }, true));

        auto v2 = v.deepcopy();
        v2.reverse();
        REQUIRE(v2.collect() == std::vector<int>{5, 4, 3, 2, 1});
        v2.filter([](int a) { return a % 2 == 0; });
        REQUIRE(v2.collect() == std::vector<int>{4, 2});
        v2.clear();
        REQUIRE(v2.collect().empty());
        v2.append(1);
        REQUIRE(v2.collect() == std::vector<int>{1});
        v2.extend({2, 3, 4, 5});
        REQUIRE(v2.collect() == std::vector<int>{1, 2, 3, 4, 5});
        v2.append(3);
        v2.remove(3);
        REQUIRE(v2.collect() == std::vector<int>{1, 2, 4, 5, 3});

        pyvec<int> tmp{};
        tmp.extend(v2);
        REQUIRE(tmp.collect() == std::vector<int>{1, 2, 4, 5, 3});
        tmp.extend(v2);
        REQUIRE(tmp.collect() == std::vector<int>{1, 2, 4, 5, 3, 1, 2, 4, 5, 3});
        tmp.clear();
        REQUIRE(tmp.collect().empty());
        tmp.append(2);
        REQUIRE(tmp.collect() == std::vector<int>{2});
        tmp.clear();
        tmp.extend({3, 4, 5});
        REQUIRE(tmp.collect() == std::vector<int>{3, 4, 5});
        tmp.extend({});
        REQUIRE(tmp.collect() == std::vector<int>{3, 4, 5});
        tmp.clear();
        REQUIRE(tmp.collect().empty());
        tmp.extend({});
        REQUIRE(tmp.collect().empty());
    }

    SECTION("magic methods") {
        auto a = v.getitem(2);
        REQUIRE(*a == 3);
        auto s1 = v.getitem({1, 4, 1});
        REQUIRE(s1.collect() == std::vector<int>{2, 3, 4});
        auto s2 = v.getitem({1, 4, 2});
        REQUIRE(s2.collect() == std::vector<int>{2, 4});
        auto s3 = v.getitem({nullopt, nullopt, nullopt});
        REQUIRE(s3.collect() == std::vector<int>{1, 2, 3, 4, 5});
        auto s4 = v.getitem({nullopt, 4, nullopt});
        REQUIRE(s4.collect() == std::vector<int>{1, 2, 3, 4});
        auto s5 = v.getitem({1, nullopt, nullopt});
        REQUIRE(s5.collect() == std::vector<int>{2, 3, 4, 5});
        auto s6 = v.getitem({nullopt, -1, nullopt});
        REQUIRE(s6.collect() == std::vector<int>{1, 2, 3, 4});
        auto s7 = v.getitem({-3, -1, nullopt});
        REQUIRE(s7.collect() == std::vector<int>{3, 4});
        auto s8 = v.getitem({nullopt, nullopt, -1});
        REQUIRE(s8.collect() == std::vector<int>{5, 4, 3, 2, 1});

        // out of range slice
        REQUIRE(v.getitem({10, 20, 1}).collect().empty());
        REQUIRE(v.getitem({10, 20, -1}).collect().empty());
        REQUIRE(v.getitem({-20, -10, 1}).collect().empty());
        REQUIRE(v.getitem({-20, -10, -1}).collect().empty());
        REQUIRE(v.getitem({1, 10, 2}).collect() == std::vector<int>{2, 4});
        REQUIRE(v.getitem({1, 10, -2}).collect().empty());
        REQUIRE(v.getitem({-10, 3, nullopt}).collect() == std::vector<int>{1, 2, 3});
        REQUIRE(v.getitem({-1, 10, nullopt}).collect() == std::vector<int>{5});
        REQUIRE(v.getitem({-1, -10, -1}).collect() == std::vector<int>{5, 4, 3, 2, 1});

        v.setitem(2, std::make_shared<int>(6));
        REQUIRE(v.collect() == std::vector<int>{1, 2, 6, 4, 5});

        // if step is not 1, the size of the input vector should be the same as the size of the
        // slice
        v.setitem({0, 4, 2}, {10, 20});
        REQUIRE(v.collect() == std::vector<int>{10, 2, 20, 4, 5});

        // if step is 1, the size of the input vector should be the same as the size of the slice
        v.setitem({0, 1, 1}, {10, 20, 30, 40});
        REQUIRE(v.collect() == std::vector<int>{10, 20, 30, 40, 2, 20, 4, 5});

        v.setitem({1, 1, 1}, {11, 12});
        REQUIRE(v.collect() == std::vector<int>{10, 11, 12, 20, 30, 40, 2, 20, 4, 5});

        v.setitem({-1, 0, -1}, {1, 2, 3, 4, 5, 6, 7, 8, 9});
        REQUIRE(v.collect() == std::vector<int>{10, 9, 8, 7, 6, 5, 4, 3, 2, 1});

        v.setitem({nullopt, nullopt, nullopt}, {1, 2, 3, 4, 5});
        REQUIRE(v.collect() == std::vector<int>{1, 2, 3, 4, 5});

        v.setitem({nullopt, nullopt, -1}, {1, 2, 3, 4, 5});
        REQUIRE(v.collect() == std::vector<int>{5, 4, 3, 2, 1});

        v.delitem(2);
        REQUIRE(v.collect() == std::vector<int>{5, 4, 2, 1});

        v.delitem({1, 3, nullopt});
        REQUIRE(v.collect() == std::vector<int>{5, 1});
        v.delitem({nullopt, nullopt, -1});
        REQUIRE(v.collect().empty());
    }
}