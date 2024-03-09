#pragma once
#ifndef PY_VEC_HPP_
#define PY_VEC_HPP_

#include <vector>
#include <memory>

template<typename T, size_t min_chunk_size = 64>
class pyvec {
public:
    using value_type = T;
    using reference = T&;
    using const_reference = const T&;
    using pointer = T*;
    using const_pointer = const T*;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;

private:
    using self = pyvec;
    using const_sef = const pyvec;

    template<typename U>
    using vec = std::vector<U>;
    template<typename U>
    using shared = std::shared_ptr<U>;


    /*
     *  Data
     */
    shared<vec<vec<T>>> _resources;
    vec<pointer> _ptrs;
    size_type _chunk_pivot = 0;
    size_type _capacity = 0;

    vec<T>& new_chunck(size_type size) {
        _resources->emplace_back();
        auto &chunck = _resources->back();
        chunck.reserve(size);
        _capacity += size;
        return chunck;
    }

    [[nodiscard]] vec<T>& suitable_chunck(size_type expected_size) {
        if (expected_size == 0) { throw std::invalid_argument("pyvec: expected_size == 0"); }
        vec<T> * ans = nullptr;
        for(auto i = _chunk_pivot; i < _resources->size(); ++i) {
            auto &chunck = _resources->operator[](i);
            const auto remaining = chunck.capacity() - chunck.size();
            if(remaining >= expected_size) {
                ans = &chunck;
                break;
            }
        }
        if(ans == nullptr) {
            const auto expanded = std::max(expected_size, std::max(_capacity, min_chunk_size));
            ans = &new_chunck(expanded);
        }
        _chunk_pivot = expected_size == 1 ? ans - _resources->data() : _chunk_pivot;
        return *ans;
    }


    [[nodiscard]] size_type py_index(difference_type i) const {
        i = i < 0 ? size() + i : i;
        if(i < 0 | i >= size()) { throw std::out_of_range("pyvec: index out of range"); }
        return static_cast<size_type>(i);
    }



public:
    /*
     *  Iterators
     */
    class iterator {
        pointer* _ptr;
    public:
        using value_type = T;
        using reference = T&;
        using pointer = T*;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::random_access_iterator_tag;

        // iterator constructor
        explicit iterator(pointer* ptr) : _ptr(ptr) {}

        // iterator dereference
        reference operator*() const { return **_ptr; }
        pointer operator->() const { return *_ptr; }

        // iterator arithmetic
        iterator& operator+=(difference_type i) { _ptr += i; return *this; }
        iterator& operator-=(difference_type i) { _ptr -= i; return *this; }
        iterator& operator++() { ++_ptr; return *this; }
        iterator& operator--() { --_ptr; return *this; }
        iterator operator++(int) { iterator tmp = *this; ++_ptr; return tmp; }
        iterator operator--(int) { iterator tmp = *this; --_ptr; return tmp; }
        difference_type operator-(const iterator& other) const { return _ptr - other._ptr; }

        // operator <=>
        bool operator==(const iterator& other) const { return _ptr == other._ptr; }
        bool operator!=(const iterator& other) const { return _ptr != other._ptr; }
        bool operator<(const iterator& other) const { return _ptr < other._ptr; }
        bool operator>(const iterator& other) const { return _ptr > other._ptr; }
        bool operator<=(const iterator& other) const { return _ptr <= other._ptr; }
        bool operator>=(const iterator& other) const { return _ptr >= other._ptr; }
    };

    class const_iterator {
        const const_pointer* _ptr;
    public:
        using value_type = T;
        using reference = const T&;
        using pointer = const T*;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::random_access_iterator_tag;

        // iterator constructor
        explicit const_iterator(const const_pointer* ptr) : _ptr(ptr) {}

        // iterator dereference
        reference operator*() const { return **_ptr; }
        pointer operator->() const { return *_ptr; }

        // iterator arithmetic
        const_iterator& operator+=(difference_type i) { _ptr += i; return *this; }
        const_iterator& operator-=(difference_type i) { _ptr -= i; return *this; }
        const_iterator& operator++() { ++_ptr; return *this; }
        const_iterator& operator--() { --_ptr; return *this; }
        const_iterator operator++(int) { const_iterator tmp = *this; ++_ptr; return tmp; }
        const_iterator operator--(int) { const_iterator tmp = *this; --_ptr; return tmp; }
        difference_type operator-(const const_iterator& other) const { return _ptr - other._ptr; }

        // operator <=>
        bool operator==(const const_iterator& other) const { return _ptr == other._ptr; }
        bool operator!=(const const_iterator& other) const { return _ptr != other._ptr; }
        bool operator<(const const_iterator& other) const { return _ptr < other._ptr; }
        bool operator>(const const_iterator& other) const { return _ptr > other._ptr; }
        bool operator<=(const const_iterator& other) const { return _ptr <= other._ptr; }
        bool operator>=(const const_iterator& other) const { return _ptr >= other._ptr; }
    };

    /*
     *  Simple Member functions
     */

    iterator begin() { return { _ptrs.data() }; }
    iterator end() { return { _ptrs.data() + _ptrs.size() }; }

    const_iterator cbegin() const { return { _ptrs.data() }; }
    const_iterator cend() const { return { _ptrs.data() + _ptrs.size() }; }

    iterator begin() const { return cbegin(); }
    iterator end() const { return cend(); }

    [[nodiscard]] size_type size() const { return _ptrs.size(); }
    [[nodiscard]] size_type capacity() const { return _capacity; }

    [[nodiscard]] bool empty() const { return _ptrs.empty(); }
    [[nodiscard]] bool inited() const { return _resources != nullptr; }

    void swap(pyvec& other) noexcept {
        std::swap(_resources, other._resources);
        std::swap(_ptrs, other._ptrs);
    }

    /*
     *  Constructors
     */

    // default constructor
    pyvec(): _resources(std::make_shared<vec<vec<T>>>()) {}

    // deepcopy constructor
    template<template<class> class Iter>
    pyvec(const Iter<T> begin, const Iter<T> end): pyvec() {
        static_assert(std::is_same_v<Iter<T>, const_iterator> || std::is_same_v<Iter<T>, iterator>, "pyvec: Invalid iterator type");
        if(begin > end) { throw std::invalid_argument("pyvec: begin > end"); }
        if(begin == end) { return; }
        const auto other_size = end - begin;
        _ptrs.reserve(other_size);
        vec<T>& chunck = new_chunck(other_size);
        for(auto it = begin; it != end; ++it) {
            chunck.push_back(*it);
            _ptrs.push_back(&chunck.back());
        }
    }

    pyvec(const pyvec& other): pyvec(other.cbegin(), other.cend()) {}

    // move constructor
    pyvec(pyvec &&other)  noexcept {
        _resources = std::move(other._resources);
        _ptrs = std::move(other._ptrs);
    }
    // destructor
    ~pyvec() = default;

    // python like slice but using deep copy to avoid potential circular references
    self slice(const difference_type begin, const difference_type end) const {
        size_type _begin = py_index(begin), _end = py_index(end);
        return self(cbegin() + _begin, cbegin() + _end);
    }

    /*
     *  Assignment
     */

    pyvec& operator=(pyvec other) { swap(other); return *this; }
    pyvec& operator=(pyvec&& other) noexcept {
        _resources = std::move(other._resources);
        _ptrs = std::move(other._ptrs);
        return *this;
    }

    /*
     *   Access
     */

    // non-const access
    reference operator[](size_type i) { return *_ptrs[i]; }
    reference at(size_type i) { return *_ptrs.at(i); }
    reference pyat(const difference_type i) { return *_ptrs[py_index(i)]; }
    shared<T> share(const difference_type i) {
        return std::make_shared<T>(_resources, _ptrs[py_index(i)]);
    }

    // const access
    const_reference operator[](size_type i) const { return *_ptrs[i]; }
    const_reference at(size_type i) const { return *_ptrs.at(i); }
    const_reference pyat(const difference_type i) const { return *_ptrs[py_index(i)]; }
    shared<const T> share(const difference_type i) const {
        return std::make_shared<const T>(_resources, _ptrs[py_index(i)]);
    }

    /*
     *  Modifiers
     */

    void push_back(const_reference value) {
        auto& chunk = suitable_chunck(1);
        chunk.push_back(value);
        _ptrs.push_back(&chunk.back());
    }




};

#endif // PY_VEC_HPP_