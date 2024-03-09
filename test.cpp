//
// Created by lyk on 24-3-9.
//
#include "pyvec.hpp"

#include <catch2/catch_test_macros.hpp>


TEST_CASE("pyvec basic editing", "[pyvec]") {
    pyvec<int> v { 1, 2, 3, 4, 5 };
    REQUIRE(v.size() == 5);
    REQUIRE(v.capacity() >= 5);
    REQUIRE(v.collect() == std::vector<int>{1, 2, 3, 4, 5});

    SECTION("push_back and emplace_back") {
        v.push_back(6);
        REQUIRE(v.size() == 6);
        REQUIRE(v.back() == 6);

        v.emplace_back(7);
        REQUIRE(v.size() == 7);
        REQUIRE(v.back() == 7);
    }

    SECTION("insert and emplace") {
        auto iter = v.emplace(v.begin() + 2, 6);
        REQUIRE(*iter == 6);
        REQUIRE(v.collect() == std::vector<int>{1,2,6,3,4,5});
    }

    SECTION("swap") {
        pyvec<int> tmp {1,2,3,-1};
        v.swap(tmp);
        REQUIRE(v.collect() == std::vector<int>{1,2,3,-1});
        REQUIRE(tmp.collect() == std::vector<int>{1,2,3,4,5});
    }

    SECTION("erase and pop_back") {
        auto it = v.erase(v.begin() + 2);
        REQUIRE(*it == 4);
        REQUIRE(v.collect() == std::vector<int>{1,2,4,5});
        it = v.erase(v.begin() + 1, v.begin() + 3);
        REQUIRE(*it == 5);
        REQUIRE(v.collect() == std::vector<int>{1,5});
        v.pop_back();
        REQUIRE(v.collect() == std::vector<int>{1});
    }

    SECTION("assign") {
        v.assign(3, 6);
        REQUIRE(v.collect() == std::vector<int>{6,6,6});
        v.assign({1,2,3,4,5});
        REQUIRE(v.collect() == std::vector<int>{1,2,3,4,5});
        pyvec<int> tmp {6,6,6};
        v.assign(tmp.begin(), tmp.end());
        REQUIRE(v.collect() == std::vector<int>{6,6,6});

        tmp.assign(5, 2);
        v = tmp;
        REQUIRE(v.collect() == std::vector<int>{2,2,2,2,2});
        tmp.assign(6, 1);
        v = std::move(tmp);
        REQUIRE(v.collect() == std::vector<int>{1,1,1,1,1,1});
        v = {1,2,3,4,5};
        REQUIRE(v.collect() == std::vector<int>{1,2,3,4,5});
    }
}

TEST_CASE("Comparison Operator", "[pyvec]") {
    pyvec<int> v1 {1,2,3,4,5};
    pyvec<int> v2 {1,2,3,4,5};
    pyvec<int> v3 {1,2,3,4,6};
    pyvec<int> v4 {1,2,3,4};
    pyvec<int> v5 {1,2,3,4,5,6};

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