#pragma once
#ifndef PYVEC_HPP
#define PYVEC_HPP

#include <vector>
#include <memory>
#include <stdexcept>
#include <optional>
#include <string>
#include "timsort.hpp"

namespace pycontainer {
struct slice {
    std::optional<ptrdiff_t> start, stop, step;

    slice(
        const std::optional<ptrdiff_t> start,
        const std::optional<ptrdiff_t> stop,
        const std::optional<ptrdiff_t> step = std::nullopt
    ) : start(start), stop(stop), step(step) {}
};

template<class InputIt>
using is_input_iterator_t = std::enable_if_t<
    std::is_base_of_v<
        std::input_iterator_tag,
        typename std::iterator_traits<InputIt>::iterator_category>,
    InputIt>;

template<class InputIt>
using is_random_access_iterator_t = std::enable_if_t<
    std::is_base_of_v<
        std::random_access_iterator_tag,
        typename std::iterator_traits<InputIt>::iterator_category>,
    InputIt>;

template<typename T>
class pyvec {
public:
    using value_type      = T;
    using reference       = T&;
    using const_reference = const T&;
    using pointer         = T*;
    using const_pointer   = const T*;
    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;

private:
    template<typename U>
    using vec = std::vector<U>;
    template<typename U>
    using shared = std::shared_ptr<U>;

    /*
     *  Private Data
     */
    static constexpr size_t min_chunk_size = 64;

    shared<vec<vec<T>>> _resources;
    vec<pointer>        _ptrs;
    shared<size_type>   _capacity;
    size_type           _chunk_pivot = 0;
    vec<T>*             _last_chunk  = nullptr;


    struct slice_native {
        size_t    start, num_steps;
        ptrdiff_t step;

        slice_native(size_t start, size_t num_steps, ptrdiff_t step) :
            start(start), num_steps(num_steps), step(step) {}
    };

public:
    /*
     *  Iterator Declaration
     */

    class iterator;
    class const_iterator;
    class shared_iterator;
    using pointer_iterator         = typename std::vector<pointer>::iterator;
    using reverse_iterator         = std::reverse_iterator<iterator>;
    using reverse_pointer_iterator = std::reverse_iterator<pointer_iterator>;

    /*
     *  Pyhton-List-Like Interface
     */
    void append(const T& value);
    void append(const shared<T>& value);

    size_type count(const T& value) const;
    size_type count(const shared<T>& value) const;

    // deepcopy when extend
    template<typename InputIt>
    void extend(is_input_iterator_t<InputIt> first, InputIt last);
    void extend(const pyvec<T>& other);

    void insert(difference_type index, const T& value);
    void insert(difference_type index, const shared<T>& value);

    shared<T> pop(difference_type index = -1);

    void remove(const T& value);
    void remove(const shared<T>& value);

    void reverse();

    void clear();

    pyvec<T> copy();

    pyvec<T> deepcopy();

    void sort(bool reverse = false);
    template<typename Key>
    void sort(Key key, bool reverse);
    template<typename Key>
    void sort_shared(Key key, bool reverse);

    [[nodiscard]] bool is_sorted(bool reverse = false) const;
    template<typename Key>
    [[nodiscard]] bool is_sorted(Key key, bool reverse) const;
    template<typename Key>
    [[nodiscard]] bool is_sorted_shared(Key key, bool reverse) const;

    template<typename Func>
    void filter(Func func);

    template<typename Func>
    void filter_shared(Func func);

    size_t index(
        const T&                       value,
        std::optional<difference_type> start = std::nullopt,
        std::optional<difference_type> stop  = std::nullopt
    ) const;

    size_t index(
        const shared<T>&               value,
        std::optional<difference_type> start = std::nullopt,
        std::optional<difference_type> stop  = std::nullopt
    ) const;

    /*
     *  Python Magic Method
     *  __getitem__, __setitem__, __delitem__, __contains__
     */

    void setitem(difference_type index, const T& value);
    void setitem(difference_type index, const shared<T>& value);

    template<typename InputIt>
    void setitem(const slice& t_slice, is_input_iterator_t<InputIt> first, InputIt last);
    void setitem(const slice& t_slice, const pyvec<T>& other);

    shared<T> getitem(difference_type index);

    // shallow copy when slicing
    pyvec<T> getitem(const slice& t_slice);

    void delitem(difference_type index);

    void delitem(const slice& t_slice);

    bool contains(const T& value) const;
    bool contains(const shared<T>& value) const;

    /*
     *  C++-Vector-Like Interface
     */

    // default constructor
    pyvec();

    // constructor with iterators
    template<typename InputIt>
    pyvec(is_input_iterator_t<InputIt> first, InputIt last);

    // deep copy constructor
    pyvec(const pyvec<T>& other);

    explicit pyvec(const vec<T>& other);

    // move constructor
    pyvec(pyvec<T>&& other) noexcept;

    explicit pyvec(vec<T>&& other);

    // initializer list constructor
    pyvec(std::initializer_list<T> il);

    // destructor
    ~pyvec() = default;

    // operator =
    pyvec<T>& operator=(const pyvec<T>& other);
    pyvec<T>& operator=(pyvec<T>&& other) noexcept;
    pyvec<T>& operator=(std::initializer_list<T> il);

    // assign
    void assign(size_type count, const T& value);

    template<class InputIt>
    void assign(is_input_iterator_t<InputIt> first, InputIt last);

    void assign(std::initializer_list<T> il);

    /*
     *  Vector-Like Element Access
     */

    reference       at(size_type pos);
    const_reference at(size_type pos) const;

    reference       operator[](size_type pos);
    const_reference operator[](size_type pos) const;

    reference       front();
    const_reference front() const;

    reference       back();
    const_reference back() const;

    // data() is removed since pyvec is not contiguous

    /*
     *  Vector-Like Iterators
     */

    iterator                 begin();
    const_iterator           begin() const;
    const_iterator           cbegin() const;
    shared_iterator          sbegin();
    pointer_iterator         pbegin();
    reverse_iterator         rbegin();
    reverse_pointer_iterator rpbegin();


    iterator                 end();
    const_iterator           end() const;
    const_iterator           cend() const;
    shared_iterator          send();
    pointer_iterator         pend();
    reverse_iterator         rend();
    reverse_pointer_iterator rpend();

    /*
     *  Vector-Like Capacity
     */

    [[nodiscard]] bool empty() const;

    [[nodiscard]] size_type size() const;

    void reserve(size_type new_cap);

    [[nodiscard]] size_type capacity() const;

    void shrink_to_fit();

    /*
     *  Vector-Like Modifiers
     */

    // clear() is declared in Python-List-Like Interface
    iterator insert(const_iterator pos, const T& value);
    iterator insert(const_iterator pos, T&& value);
    iterator insert(const_iterator pos, size_type count, const T& value);

    template<class InputIt>
    iterator insert(const_iterator pos, is_input_iterator_t<InputIt> first, InputIt last);

    iterator insert(const_iterator pos, std::initializer_list<T> il);

    // emplace
    template<class... Args>
    iterator emplace(const_iterator pos, Args&&... args);

    // erase
    iterator erase(const_iterator pos);
    iterator erase(const_iterator first, const_iterator last);

    // push_back
    void push_back(const T& value);
    void push_back(T&& value);

    // emplace_back
    template<class... Args>
    reference emplace_back(Args&&... args);

    // pop_back
    void pop_back();

    // resize
    void resize(size_type count);
    void resize(size_type count, const T& value);

    // swap
    void swap(pyvec<T>& other) noexcept;

    /*
     *  Comparison
     */

    bool operator==(const pyvec<T>& other) const;
    bool operator!=(const pyvec<T>& other) const;
    bool operator<(const pyvec<T>& other) const;
    bool operator<=(const pyvec<T>& other) const;
    bool operator>(const pyvec<T>& other) const;
    bool operator>=(const pyvec<T>& other) const;

    /*
     *  Pyvec Specific Functions
     */

    vec<T> collect() const;

private:
    /*
     *  Internal Helper Functions
     */
    void move_assign(pyvec<T>&& other);
    void move_assign(vec<T>&& other);

    void try_init();

    vec<T>& new_chunk(size_type n);

    vec<T>& add_chunk(vec<T>&& chunk);

    template<typename... Args>
    vec<T>& emplace_chunk(Args&&... args);

    [[nodiscard]] vec<T>& suitable_chunk(size_type expected_size);

    size_type insert_empty(const_iterator pos, size_type count);

    [[nodiscard]] size_type pypos(difference_type index) const;

    shared<T> share(size_type index);

    slice_native build_slice(const slice& t_slice) const;
};

// Iterator Definition
template<typename T>
class pyvec<T>::iterator {
    friend class pyvec;
    friend class const_iterator;
    pointer* _ptr;

public:
    using value_type        = T;
    using different_type    = std::ptrdiff_t;
    using iterator_category = std::random_access_iterator_tag;

    iterator() = default;

    iterator(const iterator&) = default;

    iterator(iterator&&) noexcept = default;

    // iterator constructor
    explicit iterator(pointer* ptr) : _ptr(ptr) {}

    iterator& operator=(const iterator&) = default;

    iterator& operator=(iterator&&) noexcept = default;

    // iterator dereference
    reference operator*() const { return **_ptr; }
    pointer   operator->() const { return *_ptr; }

    // iterator arithmetic
    iterator& operator+=(difference_type i) {
        _ptr += i;
        return *this;
    }

    iterator& operator-=(difference_type i) {
        _ptr -= i;
        return *this;
    }

    iterator& operator++() {
        ++_ptr;
        return *this;
    }

    iterator& operator--() {
        --_ptr;
        return *this;
    }

    iterator operator++(int) {
        iterator tmp = *this;
        ++_ptr;
        return tmp;
    }

    iterator operator--(int) {
        iterator tmp = *this;
        --_ptr;
        return tmp;
    }

    iterator operator+(difference_type i) const { return iterator{_ptr + i}; }

    iterator operator-(difference_type i) const { return iterator{_ptr - i}; }

    difference_type operator-(const iterator& other) const { return _ptr - other._ptr; }

    // operator <=>
    bool operator==(const iterator& other) const { return _ptr == other._ptr; }
    bool operator!=(const iterator& other) const { return _ptr != other._ptr; }
    bool operator<(const iterator& other) const { return _ptr < other._ptr; }
    bool operator>(const iterator& other) const { return _ptr > other._ptr; }
    bool operator<=(const iterator& other) const { return _ptr <= other._ptr; }
    bool operator>=(const iterator& other) const { return _ptr >= other._ptr; }
};

template<typename T>
class pyvec<T>::const_iterator {
    friend class pyvec;
    const const_pointer* _ptr;

public:
    using value_type        = T;
    using difference_type   = std::ptrdiff_t;
    using iterator_category = std::random_access_iterator_tag;

    const_iterator() = default;

    const_iterator(const const_iterator&) = default;

    const_iterator(const_iterator&&) noexcept = default;

    // iterator constructor
    explicit const_iterator(const const_pointer* ptr) : _ptr(ptr) {}

    // allow implicit conversion from iterator to const_iterator
    const_iterator(const iterator& other) : _ptr(other._ptr) {}   // NOLINT

    const_iterator(iterator&& other) : _ptr(other._ptr) { other._ptr = nullptr; }   // NOLINT

    const_iterator& operator=(const const_iterator&) = default;

    const_iterator& operator=(const_iterator&&) noexcept = default;

    const_iterator& operator=(const iterator& other) {
        _ptr = other._ptr;
        return *this;
    }

    const_iterator& operator=(iterator&& other) {
        _ptr       = other._ptr;
        other._ptr = nullptr;
        return *this;
    }

    // iterator dereference
    const_reference operator*() const { return **_ptr; }
    const_pointer   operator->() const { return *_ptr; }

    // iterator arithmetic
    const_iterator& operator+=(difference_type i) {
        _ptr += i;
        return *this;
    }

    const_iterator& operator-=(difference_type i) {
        _ptr -= i;
        return *this;
    }

    const_iterator& operator++() {
        ++_ptr;
        return *this;
    }

    const_iterator& operator--() {
        --_ptr;
        return *this;
    }

    const_iterator operator++(int) {
        const_iterator tmp = *this;
        ++_ptr;
        return tmp;
    }

    const_iterator operator--(int) {
        const_iterator tmp = *this;
        --_ptr;
        return tmp;
    }

    const_iterator operator+(difference_type i) const { return const_iterator{_ptr + i}; }

    const_iterator operator-(difference_type i) const { return const_iterator{_ptr - i}; }

    difference_type operator-(const const_iterator& other) const { return _ptr - other._ptr; }

    // operator <=>
    bool operator==(const const_iterator& other) const { return _ptr == other._ptr; }
    bool operator!=(const const_iterator& other) const { return _ptr != other._ptr; }
    bool operator<(const const_iterator& other) const { return _ptr < other._ptr; }
    bool operator>(const const_iterator& other) const { return _ptr > other._ptr; }
    bool operator<=(const const_iterator& other) const { return _ptr <= other._ptr; }
    bool operator>=(const const_iterator& other) const { return _ptr >= other._ptr; }
};

template<typename T>
class pyvec<T>::shared_iterator {
    friend class pyvec;
    pointer*            _ptr;
    shared<vec<vec<T>>> _resources;

public:
    using value_type        = T;
    using reference         = shared<T>;
    using difference_type   = std::ptrdiff_t;
    using iterator_category = std::random_access_iterator_tag;

    shared_iterator()                           = default;
    shared_iterator(const shared_iterator&)     = default;
    shared_iterator(shared_iterator&&) noexcept = default;
    shared_iterator(pointer* ptr, const shared<vec<vec<T>>>& resources) :
        _ptr(ptr), _resources(resources) {}

    shared_iterator& operator=(const shared_iterator&)     = default;
    shared_iterator& operator=(shared_iterator&&) noexcept = default;

    reference operator*() const { return shared<T>(_resources, *_ptr); }
    pointer   operator->() const { return *_ptr; }

    shared_iterator& operator+=(difference_type i) {
        _ptr += i;
        return *this;
    }

    shared_iterator& operator-=(difference_type i) {
        _ptr -= i;
        return *this;
    }

    shared_iterator& operator++() {
        ++_ptr;
        return *this;
    }

    shared_iterator& operator--() {
        --_ptr;
        return *this;
    }

    shared_iterator operator++(int) {
        shared_iterator tmp = *this;
        ++_ptr;
        return tmp;
    }

    shared_iterator operator--(int) {
        shared_iterator tmp = *this;
        --_ptr;
        return tmp;
    }

    shared_iterator operator+(difference_type i) const {
        return shared_iterator{_ptr + i, _resources};
    }

    shared_iterator operator-(difference_type i) const {
        return shared_iterator{_ptr - i, _resources};
    }

    difference_type operator-(const shared_iterator& other) const { return _ptr - other._ptr; }

    // operator <=>
    bool operator==(const shared_iterator& other) const { return _ptr == other._ptr; }
    bool operator!=(const shared_iterator& other) const { return _ptr != other._ptr; }
    bool operator<(const shared_iterator& other) const { return _ptr < other._ptr; }
    bool operator>(const shared_iterator& other) const { return _ptr > other._ptr; }
    bool operator<=(const shared_iterator& other) const { return _ptr <= other._ptr; }
    bool operator>=(const shared_iterator& other) const { return _ptr >= other._ptr; }
};

/*
 *  Helper Functions
 */

template<typename T>
void pyvec<T>::move_assign(pyvec<T>&& other) {
    _resources   = std::move(other._resources);
    _ptrs        = std::move(other._ptrs);
    _chunk_pivot = other._chunk_pivot;
    _capacity    = std::move(other._capacity);
    _last_chunk  = other._last_chunk;
}

template<typename T>
void pyvec<T>::move_assign(vec<T>&& other) {
    try_init();
    auto& chunk = emplace_chunk(std::move(other));
    _ptrs.resize(chunk.size());
    auto       ptr = chunk.data();
    const auto end = _ptrs.data() + _ptrs.size();
    for (auto target = _ptrs.data(); target != end; ++target) { *target = ptr++; }
}

template<typename T>
void pyvec<T>::try_init() {
    if (!_resources) { _resources = std::make_shared<vec<vec<T>>>(); }
    if (!_capacity) { _capacity = std::make_shared<size_type>(0); }
}

template<typename T>
std::vector<T>& pyvec<T>::new_chunk(size_type n) {
    _resources->emplace_back();
    auto& chunk = _resources->back();
    chunk.reserve(n);
    *_capacity += n;
    return chunk;
}

template<typename T>
std::vector<T>& pyvec<T>::add_chunk(vec<T>&& chunk) {
    *_capacity += chunk.capacity();
    _resources->emplace_back(std::move(chunk));
    return _resources->back();
}

template<typename T>
template<typename... Args>
std::vector<T>& pyvec<T>::emplace_chunk(Args&&... args) {
    _resources->emplace_back(std::forward<Args>(args)...);
    *_capacity += _resources->back().capacity();
    return _resources->back();
}

template<typename T>
std::vector<T>& pyvec<T>::suitable_chunk(size_type expected_size) {
    if (expected_size == 0) { throw std::invalid_argument("pyvec: expected_size == 0"); }
    if (_last_chunk != nullptr) {
        const auto remaining = _last_chunk->capacity() - _last_chunk->size();
        if (remaining >= expected_size) { return *_last_chunk; }
    } else {
        try_init();
    }

    vec<T>*    ans    = nullptr;
    bool       update = true;
    const auto end    = _resources->end();
    const auto begin  = _resources->begin();

    for (auto iter = _resources->begin() + _chunk_pivot; iter != end; ++iter) {
        vec<T>&         chunk     = *iter;
        const size_type remaining = chunk.capacity() - chunk.size();
        _chunk_pivot              = (update & (remaining == 0)) ? iter - begin + 1 : _chunk_pivot;
        if (remaining >= expected_size) {
            ans = &chunk;
            break;
        } else if (remaining > 0) {
            update = false;
        }
    }
    if (ans == nullptr) {
        const auto expanded = std::max(expected_size, std::max(*_capacity, min_chunk_size));
        ans                 = &new_chunk(expanded);
    }
    _last_chunk = ans;
    return *ans;
}

template<typename T>
size_t pyvec<T>::insert_empty(const const_iterator pos, const size_type count) {
    const difference_type idx = std::distance(cbegin(), pos);
    if (idx > _ptrs.size()) { throw std::out_of_range("pyvec::insert_empty"); }
    const size_type raw_size = _ptrs.size();
    _ptrs.resize(_ptrs.size() + count);
    for (difference_type i = raw_size - 1; i >= idx; --i) { _ptrs[i + count] = _ptrs[i]; }
    return idx;
}

/*
 *  Constructor
 */

template<typename T>
pyvec<T>::pyvec() {
    assign({});
}

template<typename T>
template<typename InputIt>
pyvec<T>::pyvec(is_input_iterator_t<InputIt> first, InputIt last) {
    assign(first, last);
}

template<typename T>
pyvec<T>::pyvec(const std::vector<T>& other) {
    assign(other.begin(), other.end());
}

template<typename T>
pyvec<T>::pyvec(const pyvec<T>& other) {
    assign(other.begin(), other.end());
}

template<typename T>
pyvec<T>::pyvec(std::initializer_list<T> il) {
    assign(il);
}

template<typename T>
pyvec<T>::pyvec(pyvec<T>&& other) noexcept {
    move_assign(std::move(other));
}

template<typename T>
pyvec<T>::pyvec(std::vector<T>&& other) {
    move_assign(std::move(other));
}

/*
 *  Operator =
 */

template<typename T>
pyvec<T>& pyvec<T>::operator=(const pyvec<T>& other) {
    if (this == &other) { return *this; }
    assign(other.begin(), other.end());
    return *this;
}

template<typename T>
pyvec<T>& pyvec<T>::operator=(std::initializer_list<T> il) {
    assign(il);
    return *this;
}

template<typename T>
pyvec<T>& pyvec<T>::operator=(pyvec<T>&& other) noexcept {
    move_assign(std::move(other));
    return *this;
}

/*
 *  Assign
 */

template<typename T>
void pyvec<T>::assign(size_type count, const T& value) {
    try_init();
    auto& chunk = emplace_chunk(count, value);
    _ptrs.resize(chunk.size());
    auto       ptr = chunk.data();
    const auto end = _ptrs.data() + _ptrs.size();
    for (auto target = _ptrs.data(); target != end; ++target) { *target = ptr++; }
}

template<typename T>
template<class InputIt>
void pyvec<T>::assign(is_input_iterator_t<InputIt> first, InputIt last) {
    try_init();
    if(first == last) { return; }
    auto& chunk = emplace_chunk(first, last);
    _ptrs.resize(chunk.size());
    auto       ptr = chunk.data();
    const auto end = _ptrs.data() + _ptrs.size();
    for (auto target = _ptrs.data(); target != end; ++target) { *target = ptr++; }
}

template<typename T>
void pyvec<T>::assign(std::initializer_list<T> il) {
    assign(il.begin(), il.end());
}

/*
 *  Vector-Like Element Access
 */

template<typename T>
const T& pyvec<T>::at(size_t pos) const {
    if (pos >= size()) { throw std::out_of_range("pyvec::at"); }
    return *(_ptrs[pos]);
}

template<typename T>
T& pyvec<T>::at(size_t pos) {
    if (pos >= size()) { throw std::out_of_range("pyvec::at"); }
    return *(_ptrs[pos]);
}

template<typename T>
const T& pyvec<T>::operator[](size_t pos) const {
    return *(_ptrs[pos]);
}

template<typename T>
T& pyvec<T>::operator[](size_t pos) {
    return *(_ptrs[pos]);
}

template<typename T>
const T& pyvec<T>::front() const {
    return *(_ptrs.front());
}

template<typename T>
T& pyvec<T>::front() {
    return *(_ptrs.front());
}

template<typename T>
const T& pyvec<T>::back() const {
    return *(_ptrs.back());
}

template<typename T>
T& pyvec<T>::back() {
    return *(_ptrs.back());
}

/*
 *  Vector-Like Iterators
 */

template<typename T>
typename pyvec<T>::iterator pyvec<T>::begin() {
    return iterator(_ptrs.data());
}

template<typename T>
typename pyvec<T>::const_iterator pyvec<T>::begin() const {
    return const_iterator(_ptrs.data());
}

template<typename T>
typename pyvec<T>::const_iterator pyvec<T>::cbegin() const {
    return const_iterator(_ptrs.data());
}

template<typename T>
typename pyvec<T>::shared_iterator pyvec<T>::sbegin() {
    return shared_iterator(_ptrs.data(), _resources);
}

template<typename T>
typename pyvec<T>::pointer_iterator pyvec<T>::pbegin() {
    return _ptrs.begin();
}

template<typename T>
typename pyvec<T>::reverse_iterator pyvec<T>::rbegin() {
    return reverse_iterator(end());
}

template<typename T>
typename pyvec<T>::reverse_pointer_iterator pyvec<T>::rpbegin() {
    return reverse_pointer_iterator(pend());
}

template<typename T>
typename pyvec<T>::iterator pyvec<T>::end() {
    return iterator(_ptrs.data() + _ptrs.size());
}

template<typename T>
typename pyvec<T>::const_iterator pyvec<T>::end() const {
    return const_iterator(_ptrs.data() + _ptrs.size());
}

template<typename T>
typename pyvec<T>::const_iterator pyvec<T>::cend() const {
    return const_iterator(_ptrs.data() + _ptrs.size());
}

template<typename T>
typename pyvec<T>::shared_iterator pyvec<T>::send() {
    return shared_iterator(_ptrs.data() + _ptrs.size(), _resources);
}

template<typename T>
typename pyvec<T>::pointer_iterator pyvec<T>::pend() {
    return _ptrs.end();
}

template<typename T>
typename pyvec<T>::reverse_iterator pyvec<T>::rend() {
    return reverse_iterator(begin());
}

template<typename T>
typename pyvec<T>::reverse_pointer_iterator pyvec<T>::rpend() {
    return reverse_pointer_iterator(pbegin());
}

/*
 *  Vector-Like Capacity
 */

template<typename T>
bool pyvec<T>::empty() const {
    return _ptrs.empty();
}

template<typename T>
size_t pyvec<T>::size() const {
    return _ptrs.size();
}

template<typename T>
void pyvec<T>::reserve(size_type new_cap) {
    if (const auto delta = new_cap - capacity(); delta > 0) {
        _ptrs.reserve(std::max(new_cap, _ptrs.size() + delta));
        new_chunk(std::max(min_chunk_size, delta));
    }
}

template<typename T>
size_t pyvec<T>::capacity() const {
    return *_capacity;
}

template<typename T>
void pyvec<T>::shrink_to_fit() {
    try_init();
    _ptrs.shrink_to_fit();
    *_capacity = 0;
    for (auto& chunk : *_resources) {
        chunk.shrink_to_fit();
        *_capacity += chunk.capacity();
    }
}

/*
 *  Vector-Like Modifiers
 */

template<typename T>
void pyvec<T>::clear() {
    _ptrs.clear();
    _chunk_pivot = 0;
    _resources   = std::make_shared<vec<vec<T>>>();
    _capacity    = std::make_shared<size_type>(0);
    _last_chunk  = nullptr;
}

template<typename T>
typename pyvec<T>::iterator pyvec<T>::insert(const const_iterator pos, const T& value) {
    const auto idx   = insert_empty(pos, 1);
    auto&      chunk = suitable_chunk(1);
    chunk.push_back(value);
    _ptrs[idx] = &chunk.back();
    return iterator(_ptrs.data() + idx);
}

template<typename T>
typename pyvec<T>::iterator pyvec<T>::insert(const const_iterator pos, T&& value) {
    const auto idx   = insert_empty(pos, 1);
    auto&      chunk = suitable_chunk(1);
    chunk.push_back(std::move(value));
    _ptrs[idx] = &chunk.back();
    return iterator(_ptrs.data() + idx);
}

template<typename T>
typename pyvec<T>::iterator pyvec<T>::insert(
    const_iterator pos, const size_type count, const T& value
) {
    if(count == 0) { return iterator(const_cast<pointer*>(pos._ptr)); }
    const auto idx   = insert_empty(pos, count);
    auto&      chunk = suitable_chunk(count);
    // insert the new elements
    for (auto i = idx; i < idx + count; ++i) {
        chunk.push_back(value);
        _ptrs[i] = &chunk.back();
    }
    return iterator(_ptrs.data() + idx);
}

template<typename T>
typename pyvec<T>::iterator pyvec<T>::insert(
    const const_iterator pos, std::initializer_list<T> il
) {
    return insert(pos, il.begin(), il.end());
}

template<typename T>
template<class InputIt>
typename pyvec<T>::iterator pyvec<T>::insert(
    const const_iterator pos, is_input_iterator_t<InputIt> first, InputIt last
) {
    const auto count = std::distance(first, last);
    if (count == 0) { return iterator(const_cast<pointer*>(pos._ptr)); }
    auto       idx   = insert_empty(pos, count);
    auto&      chunk = suitable_chunk(count);
    size_t     pivot = idx;
    for (auto it = first; it != last; ++it) {
        chunk.push_back(*it);
        _ptrs[pivot++] = &chunk.back();
    }
    return iterator(_ptrs.data() + idx);
}

template<typename T>
template<class... Args>
typename pyvec<T>::iterator pyvec<T>::emplace(const const_iterator pos, Args&&... args) {
    const auto idx   = insert_empty(pos, 1);
    auto&      chunk = suitable_chunk(1);
    chunk.emplace_back(std::forward<Args>(args)...);
    _ptrs[idx] = &chunk.back();
    return iterator(_ptrs.data() + idx);
}

template<typename T>
typename pyvec<T>::iterator pyvec<T>::erase(const_iterator pos) {
    const auto idx = std::distance(cbegin(), pos);
    if (idx >= _ptrs.size() | idx < 0) { throw std::out_of_range("pyvec::erase"); }
    _ptrs.erase(_ptrs.begin() + idx);
    return iterator(_ptrs.data() + idx);
}

template<typename T>
typename pyvec<T>::iterator pyvec<T>::erase(const_iterator first, const_iterator last) {
    const auto left  = std::distance(cbegin(), first);
    const auto right = std::distance(cbegin(), last);
    if (left >= _ptrs.size() || left < 0 || right > _ptrs.size() || right < 0) {
        throw std::out_of_range("pyvec::erase");
    }
    _ptrs.erase(_ptrs.begin() + left, _ptrs.begin() + right);
    return iterator(_ptrs.data() + left);
}

template<typename T>
void pyvec<T>::push_back(const T& value) {
    auto& chunk = suitable_chunk(1);
    chunk.push_back(value);
    _ptrs.push_back(&chunk.back());
}

template<typename T>
void pyvec<T>::push_back(T&& value) {
    auto& chunk = suitable_chunk(1);
    chunk.push_back(std::move(value));
    _ptrs.push_back(&chunk.back());
}

template<typename T>
template<class... Args>
typename pyvec<T>::reference pyvec<T>::emplace_back(Args&&... args) {
    auto& chunk = suitable_chunk(1);
    chunk.emplace_back(std::forward<Args>(args)...);
    _ptrs.push_back(&chunk.back());
    return chunk.back();
}

template<typename T>
void pyvec<T>::pop_back() {
    if (_ptrs.empty()) { throw std::out_of_range("pyvec::pop_back"); }
    _ptrs.pop_back();
}

template<typename T>
void pyvec<T>::resize(size_type count) {
    if (count <= size()) { return _ptrs.resize(count); }
    const auto delta = count - size();
    auto&      chunk = suitable_chunk(delta);
    auto       idx   = chunk.size();
    chunk.resize(idx + delta);
    _ptrs.reserve(count);
    for (; idx < chunk.size(); ++idx) { _ptrs.push_back(&chunk[idx]); }
}

template<typename T>
void pyvec<T>::resize(size_type count, const T& value) {
    if (count <= size()) { return _ptrs.resize(count); }
    const auto delta = count - size();
    auto&      chunk = suitable_chunk(delta);
    auto       idx   = chunk.size();
    chunk.resize(idx + delta, value);
    _ptrs.reserve(count);
    for (; idx < chunk.size(); ++idx) { _ptrs.push_back(&chunk[idx]); }
}

template<typename T>
void pyvec<T>::swap(pyvec<T>& other) noexcept {
    std::swap(_resources, other._resources);
    std::swap(_ptrs, other._ptrs);
    std::swap(_chunk_pivot, other._chunk_pivot);
    std::swap(_capacity, other._capacity);
}

/*
 *  Comparison
 */
template<typename T>
bool pyvec<T>::operator==(const pyvec<T>& other) const {
    // need to check if T is comparable
    if (size() != other.size()) { return false; }
    for (auto it = cbegin(), it2 = other.cbegin(); it != cend(); ++it, ++it2) {
        if (*it != *it2) { return false; }
    }
    return true;
}

template<typename T>
bool pyvec<T>::operator!=(const pyvec<T>& other) const {
    return !(*this == other);
}

template<typename T>
bool pyvec<T>::operator<(const pyvec<T>& other) const {
    // need to check if T is comparable
    const auto min_size = std::min(size(), other.size());
    for (auto i = 0; i < min_size; ++i) {
        if (*_ptrs[i] < *other._ptrs[i]) { return true; }
        if (*other._ptrs[i] < *_ptrs[i]) { return false; }
    }
    return size() < other.size();
}

template<typename T>
bool pyvec<T>::operator<=(const pyvec<T>& other) const {
    return !(other < *this);
}

template<typename T>
bool pyvec<T>::operator>(const pyvec<T>& other) const {
    return other < *this;
}

template<typename T>
bool pyvec<T>::operator>=(const pyvec<T>& other) const {
    return !(*this < other);
}

/*
 *  Python-List-Like Interface
 */

template<typename T>
void pyvec<T>::append(const T& value) {
    push_back(value);
}

template<typename T>
void pyvec<T>::append(const shared<T>& value) {
    push_back(*value);
}

template<typename T>
size_t pyvec<T>::count(const T& value) const {
    size_t cnt = 0;
    for (auto& item : _ptrs) {
        if (*item == value) ++cnt;
    }
    return cnt;
}

template<typename T>
size_t pyvec<T>::count(const shared<T>& value) const {
    return count(*value);
}

template<typename T>
template<typename InputIt>
void pyvec<T>::extend(is_input_iterator_t<InputIt> first, InputIt last) {
    insert(cend(), first, last);
}

template<typename T>
void pyvec<T>::extend(const pyvec<T>& other) {
    insert(cend(), other.cbegin(), other.cend());
}

template<typename T>
pyvec<T> pyvec<T>::copy() {
    pyvec<T> ans{};
    ans._resources = _resources;   // shallow copy
    ans._ptrs.assign(_ptrs.begin(), _ptrs.end());
    ans._chunk_pivot = _chunk_pivot;
    ans._capacity    = _capacity;   // shallow copy
    return ans;
}

template<typename T>
pyvec<T> pyvec<T>::deepcopy() {
    return pyvec(*this);
}

template<typename T>
typename pyvec<T>::size_type pyvec<T>::pypos(difference_type index) const {
    difference_type ans = index;
    if (ans < 0) { ans += size(); }
    if (ans < 0 | ans >= size()) {
        throw std::out_of_range("pyvec::index out of range: " + std::to_string(index));
    }
    return ans;
}

template<typename T>
std::shared_ptr<T> pyvec<T>::share(size_type index) {
    T* ptr = _ptrs[index];
    return std::shared_ptr<T>(_resources, ptr);
}

template<typename T>
void pyvec<T>::insert(const difference_type index, const T& value) {
    const difference_type pos = index >= size() ? size() : pypos(index);
    insert(cbegin() + pos, value);
}

template<typename T>
void pyvec<T>::insert(const difference_type index, const shared<T>& value) {
    insert(index, *value);
}

template<typename T>
std::shared_ptr<T> pyvec<T>::pop(const difference_type index) {
    const size_type pos = pypos(index);
    shared<T>       ans = share(pos);
    _ptrs.erase(_ptrs.begin() + pos);
    return ans;
}

template<typename T>
void pyvec<T>::remove(const T& value) {
    // remove the first occurrence of value
    for (auto it = begin(); it != end(); ++it) {
        if (*it == value) {
            _ptrs.erase(_ptrs.begin() + std::distance(begin(), it));
            return;
        }
    }
    throw std::invalid_argument("pyvec::remove: value not found");
}

template<typename T>
void pyvec<T>::remove(const shared<T>& value) {
    remove(*value);
}

template<typename T>
void pyvec<T>::reverse() {
    std::reverse(_ptrs.begin(), _ptrs.end());
}

template<typename T>
void pyvec<T>::sort(const bool reverse) {
    sort([](const T& k) -> const T& { return k; }, reverse);
}

template<typename T>
template<typename Key>
void pyvec<T>::sort(Key key, const bool reverse) {
    auto cmp = [&key](const pointer& a, const pointer& b) { return key(*a) < key(*b); };
    if (reverse) {
        gfx::timsort(_ptrs.rbegin(), _ptrs.rend(), cmp);
    } else {
        gfx::timsort(_ptrs.begin(), _ptrs.end(), cmp);
    }
}

template<typename T>
template<typename Key>
void pyvec<T>::sort_shared(Key key, const bool reverse) {
    auto cmp = [&](const pointer& a, const pointer& b) {
        return key(shared<T>(_resources, a)) < key(shared<T>(_resources, b));
    };
    if (reverse) {
        gfx::timsort(_ptrs.rbegin(), _ptrs.rend(), cmp);
    } else {
        gfx::timsort(_ptrs.begin(), _ptrs.end(), cmp);
    }
}


template<typename T>
bool pyvec<T>::is_sorted(const bool reverse) const {
    return is_sorted([](const T& k) -> const T& { return k; }, reverse);
}

template<typename T>
template<typename Key>
bool pyvec<T>::is_sorted(Key key, const bool reverse) const {
    auto cmp = [&key](const pointer& a, const pointer& b) { return key(*a) < key(*b); };
    if (reverse) {
        return std::is_sorted(_ptrs.rbegin(), _ptrs.rend(), cmp);
    } else {
        return std::is_sorted(_ptrs.begin(), _ptrs.end(), cmp);
    }
}

template<typename T>
template<typename Key>
bool pyvec<T>::is_sorted_shared(Key key, const bool reverse) const {
    auto cmp = [&](const pointer& a, const pointer& b) {
        return key(shared<T>(_resources, a)) < key(shared<T>(_resources, b));
    };
    if (reverse) {
        return std::is_sorted(_ptrs.rbegin(), _ptrs.rend(), cmp);
    } else {
        return std::is_sorted(_ptrs.begin(), _ptrs.end(), cmp);
    }
}

template<typename T>
template<typename Func>
void pyvec<T>::filter(Func func) {
    auto it = std::remove_if(_ptrs.begin(), _ptrs.end(), [&func](const pointer& ptr) {
        return !func(*ptr);
    });
    _ptrs.erase(it, _ptrs.end());
}

template<typename T>
template<typename Func>
void pyvec<T>::filter_shared(Func func) {
    auto it = std::remove_if(_ptrs.begin(), _ptrs.end(), [&func, this](const pointer& ptr) {
        return !func(shared<T>(_resources, ptr));
    });
    _ptrs.erase(it, _ptrs.end());
}

template<typename T>
size_t pyvec<T>::index(
    const T&                             value,
    const std::optional<difference_type> start,
    const std::optional<difference_type> stop
) const {
    const auto      left  = pypos(start.value_or(0));
    difference_type right = stop.value_or(size());

    right = right >= size() ? size() : pypos(right);
    for (auto i = left; i < right; ++i) {
        if (*_ptrs[i] == value) { return i; }
    }
    throw std::invalid_argument("pyvec::index: value not found");
}

template<typename T>
size_t pyvec<T>::index(
    const shared<T>&                     value,
    const std::optional<difference_type> start,
    const std::optional<difference_type> stop
) const {
    return index(*value, start, stop);
}

/*
 *  Python Magic Method
 */
template<typename T>
typename pyvec<T>::slice_native pyvec<T>::build_slice(const slice& t_slice) const {
    difference_type start, stop, num_steps;
    difference_type step   = t_slice.step.value_or(1);
    auto            v_size = static_cast<difference_type>(size());
    constexpr auto  zero   = static_cast<difference_type>(0);
    if (step == 0) { throw std::invalid_argument("slice::step == 0"); }
    if (step > 0) {
        start = t_slice.start.value_or(0);
        if (start < 0) {
            start = std::max(zero, start + v_size);
        } else {
            start = std::min(start, v_size);
        }

        stop = t_slice.stop.value_or(v_size);
        if (stop < 0) {
            stop = std::max(zero, stop + v_size);
        } else {
            stop = std::min(stop, v_size);
        }

        num_steps = std::max(zero, (stop - start - 1) / step + 1);
    } else {
        start = t_slice.start.value_or(v_size - 1);
        if (start < 0) {
            start = std::max(static_cast<difference_type>(-1), start + v_size);
        } else {
            start = std::min(start, v_size - 1);
        }

        if (t_slice.stop.has_value()) {
            stop = t_slice.stop.value();
            if (stop < 0) {
                stop = std::max(static_cast<difference_type>(-1), stop + v_size);
            } else {
                stop = std::min(stop, v_size);
            }
        } else {
            stop = -1;
        }

        num_steps = std::max(zero, (start - stop - 1) / -step + 1);
    }
    return {static_cast<size_type>(start), static_cast<size_type>(num_steps), step};
}

template<typename T>
void pyvec<T>::setitem(const difference_type index, const T& value) {
    const auto pos   = pypos(index);
    auto&      chunk = suitable_chunk(1);
    chunk.push_back(value);
    _ptrs[pos] = &chunk.back();
}

template<typename T>
void pyvec<T>::setitem(const difference_type index, const shared<T>& value) {
    setitem(index, *value);
}

template<typename T>
template<typename InputIt>
void pyvec<T>::setitem(const slice& t_slice, is_input_iterator_t<InputIt> first, InputIt last) {
    auto      s          = build_slice(t_slice);
    size_type other_size = std::distance(first, last);
    if (other_size == 0) { this->delitem(t_slice); }
    if (s.step == 1) {
        difference_type delta =
            static_cast<difference_type>(other_size) - static_cast<difference_type>(s.num_steps);
        if (delta > 0) {
            insert_empty(cbegin() + s.start + s.num_steps, delta);
        } else if (delta < 0) {
            _ptrs.erase(
                _ptrs.begin() + s.start + s.num_steps + delta, _ptrs.begin() + s.start + s.num_steps
            );
        }

        auto&  chunk = suitable_chunk(other_size);
        size_t pivot = s.start;
        for (auto it = first; it != last; ++it) {
            chunk.push_back(*it);
            _ptrs[pivot++] = &chunk.back();
        }
    } else if (s.num_steps == other_size) {
        auto&  chunk = suitable_chunk(s.num_steps);
        size_t pivot = s.start;
        for (auto it = first; it != last; ++it) {
            chunk.push_back(*it);
            _ptrs[pivot] = &chunk.back();
            pivot += s.step;
        }
    } else {
        throw std::invalid_argument("pyvec::setitem: incompatible slice and sequence");
    }
}

template<typename T>
void pyvec<T>::setitem(const slice& t_slice, const pyvec<T>& other) {
    setitem(t_slice, other.cbegin(), other.cend());
}

template<typename T>
std::shared_ptr<T> pyvec<T>::getitem(const difference_type index) {
    return share(pypos(index));
}

template<typename T>
pyvec<T> pyvec<T>::getitem(const slice& t_slice) {
    auto s = build_slice(t_slice);
    if (s.num_steps == 0) { return pyvec<T>{}; }

    pyvec<T> ans;
    ans._capacity    = _capacity;
    ans._resources   = _resources;
    ans._chunk_pivot = _chunk_pivot;

    if (s.step == 1) {
        // std::copy is faster than loop
        ans._ptrs.assign(_ptrs.begin() + s.start, _ptrs.begin() + s.start + s.num_steps);
    } else {
        ans._ptrs.reserve(s.num_steps);
        size_t pivot = s.start;
        for (size_t i = 0; i < s.num_steps; ++i) {
            ans._ptrs.push_back(_ptrs[pivot]);
            pivot += s.step;
        }
    }
    return ans;
}

template<typename T>
void pyvec<T>::delitem(const difference_type index) {
    const auto pos = pypos(index);
    _ptrs.erase(_ptrs.begin() + pos);
}

template<typename T>
void pyvec<T>::delitem(const slice& t_slice) {
    auto s = build_slice(t_slice);
    if (s.num_steps == 0) { return; }
    auto new_ptrs = vec<pointer>{};
    new_ptrs.reserve(size() - s.num_steps);
    for (difference_type i = 0; i < size(); ++i) {
        const auto delta = i - static_cast<difference_type>(s.start);
        if ((delta % s.step == 0) && (delta / s.step < s.num_steps)) {
            continue;
        } else {
            new_ptrs.push_back(_ptrs[i]);
        }
    }

    _ptrs = std::move(new_ptrs);
}

template<typename T>
bool pyvec<T>::contains(const T& value) const {
    auto check = [&value](const pointer& ptr) { return *ptr == value; };
    if (std::any_of(_ptrs.begin(), _ptrs.end(), check)) { return true; }
    return false;
}

template<typename T>
bool pyvec<T>::contains(const shared<T>& value) const {
    return contains(*value);
}

/*
 *  Pyvec Specific Functions
 */

template<typename T>
std::vector<T> pyvec<T>::collect() const {
    return std::vector<T>{cbegin(), cend()};
}
}   // namespace pycontainer
#endif   // PYVEC_HPP