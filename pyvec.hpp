#pragma once
#ifndef PY_VEC_HPP_
#define PY_VEC_HPP_

#include <vector>
#include <memory>
#include <stdexcept>

template<typename T, size_t min_chunk_size = 64>
class pyvec {
public:
    using value_type = T;
    using reference = T &;
    using const_reference = const T &;
    using pointer = T *;
    using const_pointer = const T *;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;

private:
    using self = pyvec;
    using const_sef = const pyvec;

    template<typename U>
    using vec = std::vector<U>;
    template<typename U>
    using shared = std::shared_ptr<U>;

    template<class InputIt>
    using is_input_iterator_t = std::enable_if_t<std::is_base_of_v<std::input_iterator_tag, typename std::iterator_traits<InputIt>::iterator_category>, InputIt>;
    template<class InputIt>
    using is_random_access_iterator_t = std::enable_if_t<std::is_base_of_v<std::random_access_iterator_tag, typename std::iterator_traits<InputIt>::iterator_category>, InputIt>;

    /*
     *  Data
     */

    shared<vec<vec<T>>> _resources;
    vec<pointer> _ptrs;

    size_type _chunk_pivot = 0;
    size_type _capacity = 0;

    vec<decltype(_resources)> _shared_resources;

    /*
     *  Private helper functions
     */

    vec<T>& new_chunck(size_type n) {
        _resources->emplace_back();
        auto& chunck = _resources->back();
        chunck.reserve(n);
        _capacity += n;
        return chunck;
    }

    [[nodiscard]] vec<T>& suitable_chunck(size_type expected_size) {
        if (expected_size == 0) { throw std::invalid_argument("pyvec: expected_size == 0"); }
        try_init();
        vec<T>* ans = nullptr;
        for (auto i = _chunk_pivot; i < _resources->size(); ++i) {
            auto& chunck = _resources->operator[](i);
            const auto remaining = chunck.capacity() - chunck.size();
            if (remaining >= expected_size) {
                ans = &chunck;
                break;
            }
        }
        if (ans == nullptr) {
            const auto expanded = std::max(expected_size, std::max(_capacity, min_chunk_size));
            ans = &new_chunck(expanded);
        }
        _chunk_pivot = expected_size == 1 ? ans - _resources->data() : _chunk_pivot;
        return *ans;
    }

    struct slice_type {
        size_type l, r;
        size_type num_step;
        difference_type step;
        slice_type(const size_type l, const size_type r, const difference_type step): l(l), r(r), step(step) {
            if(step == 0) { throw std::invalid_argument("pyvec: step == 0"); }
            const difference_type n = static_cast<difference_type>(r - l) / step;
            num_step = n > 0? n: 0;
        }
    };


    [[nodiscard]] size_type py_index(difference_type i) const {
        i = i < 0 ? size() + i : i;
        if (i < 0 | i >= size()) { throw std::out_of_range("pyvec: index out of range"); }
        return static_cast<size_type>(i);
    }

    [[nodiscard]] slice_type py_slice(const difference_type begin, const difference_type end, const difference_type step=1) const {
        const size_type l = py_index(begin);
        const size_type r = end == _ptrs.size()? end: py_index(end);
        return slice_type{l, r, step};
    }

    void ref_other(const pyvec& other) {
        _shared_resources.reserve(other._shared_resources.size() + 1 + _shared_resources.size());
        _shared_resources.push_back(other._resources);
        for(const auto &res: other._shared_resources) {
            _shared_resources.push_back(res);
        }
    }

public:
    /*
     *  Iterators
     */
    using pointer_iterator = typename decltype(_ptrs)::iterator;

    class const_iterator;

    class iterator {
        friend class pyvec;
        friend class const_iterator;
        pointer* _ptr;

    public:

        using value_type = T;
        using reference = T &;
        using pointer = T *;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::random_access_iterator_tag;

        // iterator constructor
        explicit iterator(pointer* ptr) : _ptr(ptr) {
        }

        // iterator dereference
        reference operator*() const { return **_ptr; }
        pointer operator->() const { return *_ptr; }

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

    class const_iterator {
        friend class pyvec;
        const const_pointer* _ptr;

    public:
        using value_type = T;
        using reference = const T &;
        using pointer = const T *;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::random_access_iterator_tag;

        // iterator constructor
        explicit const_iterator(const const_pointer* ptr) : _ptr(ptr) {}

        // allow implicit conversion from iterator to const_iterator
        const_iterator(const iterator& other) : _ptr(other._ptr) {}

        // iterator dereference
        reference operator*() const { return **_ptr; }
        pointer operator->() const { return *_ptr; }

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

    iterator begin() { return iterator{_ptrs.data()}; }
    iterator end() { return iterator{_ptrs.data() + _ptrs.size()}; }

    const_iterator cbegin() const { return const_iterator{_ptrs.data()}; }
    const_iterator cend() const { return const_iterator{_ptrs.data() + _ptrs.size()}; }

    const_iterator begin() const { return cbegin(); }
    const_iterator end() const { return cend(); }

    pointer_iterator pbegin() { return _ptrs.begin(); }
    pointer_iterator pend() { return _ptrs.end(); }

    T& front() { return *_ptrs.front(); }
    T& back() { return *_ptrs.back(); }

    const T& front() const { return *_ptrs.front(); }
    const T& back() const { return *_ptrs.back(); }

    /*
     *  Capacity
     */

    [[nodiscard]] size_type size() const { return _ptrs.size(); }
    [[nodiscard]] size_type capacity() const { return _capacity; }

    [[nodiscard]] bool empty() const { return _ptrs.empty(); }
    [[nodiscard]] bool inited() const { return _resources ? true : false; }

    void resize(const size_type n) {
        const auto cur_size = size();
        _ptrs.resize(n);
        if (n > cur_size) {
            vec<T>& chunck = suitable_chunck(n - cur_size);
            const auto cur_chunck_size = chunck.size();
            const auto new_chunck_size = cur_chunck_size + n - cur_size;
            if (new_chunck_size > chunck.capacity()) {
                throw std::runtime_error("pyvec::resize: new_chunck_size > chunck.capacity()");
            }
            chunck.resize(new_chunck_size);
            for (auto i = 0; i < n - cur_size; ++i) {
                _ptrs[cur_size + i] = &chunck[cur_chunck_size + i];
            }
        }
    }

    void resize(const size_type n, const_reference value) {
        const auto cur_size = size();
        _ptrs.resize(n);
        if (n > cur_size) {
            vec<T>& chunck = suitable_chunck(n - cur_size);
            const auto cur_chunck_size = chunck.size();
            const auto new_chunck_size = cur_chunck_size + n - cur_size;
            if (new_chunck_size > chunck.capacity()) {
                throw std::runtime_error("pyvec::resize: new_chunck_size > chunck.capacity()");
            }
            chunck.resize(new_chunck_size, value);
            for (auto i = 0; i < n - cur_size; ++i) {
                chunck.push_back(value);
                _ptrs[cur_size + i] = &chunck[cur_chunck_size + i];
            }
        }
    }

    void shrink_to_fit() {
        _ptrs.shrink_to_fit();
        _resources->shrink_to_fit();
    }

    void reserve(const size_type n) {
        if (const auto delta = n - _capacity; delta > 0) {
            _ptrs.reserve(std::max(n, _ptrs.size() + delta));
            new_chunck(std::max(min_chunk_size, delta));
        }
    }

    /*
     *  Constructors
     */

    // default constructor
    pyvec(): _resources(std::make_shared<vec<vec<T>>>()), _ptrs(), _chunk_pivot(0), _capacity(0), _shared_resources() {
    }

    // // deepcopy constructor

    template<typename InputIt>
    pyvec(is_input_iterator_t<InputIt> begin, InputIt end): pyvec() {
        if (begin == end) { return; }
        if constexpr (std::is_base_of_v<std::random_access_iterator_tag, typename std::iterator_traits<InputIt>::iterator_category>) {
            const auto other_size = std::distance(begin, end);
            _ptrs.reserve(other_size);
            vec<T>& chunck = new_chunck(other_size);
            for (auto it = begin; it != end; ++it) {
                chunck.push_back(*it);
                _ptrs.push_back(&chunck.back());
            }
        } else {
            for(auto it = begin; it != end; ++it) {
                push_back(*it);
            }
        }
    }

    pyvec(const pyvec& other): pyvec(other.cbegin(), other.cend()) {}

    // move constructor
    pyvec(pyvec&& other) noexcept {
        _resources = std::move(other._resources);
        _ptrs = std::move(other._ptrs);
        _chunk_pivot = other._chunk_pivot;
        _capacity = other._capacity;
        _shared_resources = std::move(other._shared_resources);
    }

    // initializer list constructor
    pyvec(std::initializer_list<T> il): pyvec() {
        vec<T>& chunck = new_chunck(il.size());
        for (auto it = il.begin(); it != il.end(); ++it) {
            chunck.push_back(*it);
            _ptrs.push_back(&chunck.back());
        }
    }

    // const

    // destructor
    ~pyvec() = default;

    void try_init() {
        if (!_resources) {
            _resources = std::make_shared<vec<vec<T>>>();
        }
    }

    // python like slice, using shallow copy
    pyvec<T> slice(const difference_type begin, const difference_type end, const difference_type step=1) const {
        const auto t_slice = py_slice(begin, end, step);
        if(t_slice.num_step <= 0) { return pyvec<T>{}; }
        pyvec ans{};
        ans.ref_other(*this);
        ans._ptrs.reserve(t_slice.num_step);
        for(auto i = t_slice.l; i < t_slice.r; i += step) {
            ans._ptrs.push_back(_ptrs[i]);
        }
        return ans;
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

    [[nodiscard]] vec<T> collect() const {
        vec<T> ans;
        ans.reserve(size());
        for (const auto& value: *this) {
            ans.push_back(value);
        }
        return ans;
    }

    /*
     *  Modifiers
     */

    void push_back(const_reference value) {
        auto& chunk = suitable_chunck(1);
        chunk.push_back(value);
        _ptrs.push_back(&chunk.back());
    }

    void push_back(T&& value) {
        auto& chunk = suitable_chunck(1);
        chunk.push_back(std::move(value));
        _ptrs.push_back(&chunk.back());
    }

    template<typename... Args>
    iterator emplace(const_iterator pos, Args&&... args) {
        vec<T>& chunk = suitable_chunck(1);
        chunk.emplace_back(std::forward<Args>(args)...);
        const size_type index = pos - cbegin();
        _ptrs.insert(_ptrs.begin() + index, &chunk.back());
        return iterator{_ptrs.data() + index};
    }

    template<typename... Args>
    void emplace_back(Args&&... args) {
        vec<T>& chunk = suitable_chunck(1);
        chunk.emplace_back(std::forward<Args>(args)...);
        _ptrs.push_back(&chunk.back());
    }

    iterator insert(const_iterator pos, const_reference value) {
        vec<T>& chunk = suitable_chunck(1);
        chunk.push_back(value);
        const size_type index = pos - cbegin();
        _ptrs.insert(_ptrs.begin() + index, &chunk.back());
        return iterator{_ptrs.data() + index};
    }

    iterator insert(const_iterator pos, T&& value) {
        vec<T>& chunk = suitable_chunck(1);
        chunk.push_back(std::move(value));
        const size_type index = pos - cbegin();
        _ptrs.insert(_ptrs.begin() + index, &chunk.back());
        return iterator{_ptrs.data() + index};
    }

    iterator insert(const_iterator pos, size_type n, const_reference value) {
        vec<T>& chunk = suitable_chunck(n);
        const size_type index = pos - cbegin();
        _ptrs.resize(_ptrs.size() + n);
        // move the elements after pos
        for (auto i = _ptrs.size() - 1; i >= index + n; --i) {
            _ptrs[i] = _ptrs[i - n];
        }
        // insert the new elements
        for (auto i = 0; i < n; ++i) {
            chunk.push_back(value);
            _ptrs[index + i] = &chunk.back();
        }
        return iterator{_ptrs.data() + index};
    }

    iterator insert(const_iterator pos, std::initializer_list<T> il) {
        vec<T>& chunk = suitable_chunck(il.size());
        const size_type index = pos - cbegin();
        _ptrs.resize(_ptrs.size() + il.size());
        // move the elements after pos
        for (auto i = _ptrs.size() - 1; i >= index + il.size(); --i) {
            _ptrs[i] = _ptrs[i - il.size()];
        }
        // insert the new elements
        for (auto it = il.begin(); it != il.end(); ++it) {
            chunk.push_back(*it);
            _ptrs[index + (it - il.begin())] = &chunk.back();
        }
        return iterator{_ptrs.data() + index};
    }
    // using template to represent any type of iterator
    // but we need to add
    template< class InputIt>
    iterator insert(const_iterator pos, is_input_iterator_t<InputIt> first, InputIt last) {
        if(first == last) { return iterator{_ptrs.data() + (pos - cbegin())}; }
        if constexpr (!std::is_base_of_v<std::random_access_iterator_tag, typename std::iterator_traits<InputIt>::iterator_category>) {
            std::vector<T> tmp{first, last};
            return insert(pos, tmp.begin(), tmp.end());
        } else {
            const size_type n = std::distance(first, last);
            vec<T>& chunk = suitable_chunck(n);
            const size_type index = pos - cbegin();
            _ptrs.resize(_ptrs.size() + n);
            // move the elements after pos
            for (auto i = _ptrs.size() - 1; i >= index + n; --i) {
                _ptrs[i] = _ptrs[i - n];
            }
            // insert the new elements
            for (auto it = first; it != last; ++it) {
                chunk.push_back(*it);
                _ptrs[index + (it - first)] = &chunk.back();
            }
            return iterator{_ptrs.data() + index};
        }
    }


    void swap(pyvec& other) noexcept {
        std::swap(_resources, other._resources);
        std::swap(_ptrs, other._ptrs);
        std::swap(_chunk_pivot, other._chunk_pivot);
        std::swap(_capacity, other._capacity);
    }

    void clear() {
        _resources = std::make_shared<vec<vec<T>>>();
        _ptrs.clear();
        _shared_resources.clear();
        _chunk_pivot = 0;
        _capacity = 0;
    }

    iterator erase(const_iterator pos) {
        const size_type index = pos - cbegin();
        _ptrs.erase(_ptrs.begin() + index);
        return iterator{_ptrs.data() + index};
    }

    iterator erase(const_iterator begin, const_iterator end) {
        if (begin > end) { throw std::invalid_argument("pyvec: begin > end"); }
        const size_type l = begin - cbegin();
        const size_type r = end - cbegin();
        _ptrs.erase(_ptrs.begin() + l, _ptrs.begin() + r);
        return iterator{_ptrs.data() + l};
    }

    void pop_back() { _ptrs.pop_back(); }

    /*
     *  Assignment
     */

    void assign(const size_type n, const_reference value) {
        clear();
        vec<T>& chunck = suitable_chunck(n);
        for (auto i = 0; i < n; ++i) {
            chunck.push_back(value);
            _ptrs.emplace_back(&chunck.back());
        }
    }

    void assign(const_iterator begin, const_iterator end) {
        if (begin > end) { throw std::invalid_argument("pyvec: begin > end"); }
        clear();
        const size_type n = end - begin;
        vec<T>& chunck = suitable_chunck(n);
        for (auto it = begin; it != end; ++it) {
            chunck.push_back(*it);
            _ptrs.emplace_back(&chunck.back());
        }
    }

    void assign(std::initializer_list<T> il) {
        clear();
        vec<T>& chunck = suitable_chunck(il.size());
        for (auto it = il.begin(); it != il.end(); ++it) {
            chunck.push_back(*it);
            _ptrs.emplace_back(&chunck.back());
        }
    }

    pyvec& operator=(const pyvec& other) {
        if (this == &other) { return *this; }
        clear();
        vec<T>& chunck = new_chunck(other.size());
        for (const auto& value: other) {
            chunck.push_back(value);
            _ptrs.push_back(&chunck.back());
        }
        return *this;
    }

    pyvec& operator=(pyvec&& other) noexcept {
        if (this == &other) { return *this; }
        _resources = std::move(other._resources);
        _ptrs = std::move(other._ptrs);
        _chunk_pivot = other._chunk_pivot;
        _capacity = other._capacity;
        return *this;
    }

    pyvec& operator=(std::initializer_list<T> il) {
        assign(il);
        return *this;
    }

    /*
     *  Relational operators
     */

    [[nodiscard]] bool operator==(const pyvec<T>& other) const {
        // need to check if T is comparable
        if (size() != other.size()) { return false; }
        for (auto it = cbegin(), it2 = other.cbegin(); it != cend(); ++it, ++it2) {
            if (*it != *it2) { return false; }
        }
        return true;
    }

    [[nodiscard]] bool operator!=(const pyvec<T>& other) const { return !(*this == other); }

    [[nodiscard]] bool operator<(const pyvec<T>& other) const {
        // need to check if T is comparable
        for (auto it = cbegin(), it2 = other.cbegin(); it != cend() && it2 != other.cend(); ++it, ++it2) {
            if (*it < *it2) { return true; }
            if (*it > *it2) { return false; }
        }
        return size() < other.size();
    }

    [[nodiscard]] bool operator>(const pyvec<T>& other) const { return other < *this; }
    [[nodiscard]] bool operator<=(const pyvec<T>& other) const { return !(other < *this); }
    [[nodiscard]] bool operator>=(const pyvec<T>& other) const { return !(*this < other); }
    /*
     *  Python like iterface
     */
    void append(const_reference value) { push_back(value); }
    void append(T&& value) { push_back(std::move(value)); }

    void extend(const pyvec& other) {
        ref_other(other);
        _ptrs.reserve(_ptrs.size() + other.size());
        for(const auto& ptr: other._ptrs) {
            _ptrs.push_back(ptr);
        }
    }

    shared<T> getitem(const difference_type i) { return share(i); }

    pyvec<T> getitem(const difference_type begin, const difference_type end, const difference_type step=1) const {
        return slice(begin, end, step);
    }

    void delitem(const difference_type i) { erase(cbegin() + py_index(i)); }

    void delitem(const difference_type begin, const difference_type end, const difference_type step=1) {
        const auto t_slice = py_slice(begin, end, step);
        // using filter
        if(t_slice.num_step <= 0) { return; }
        std::vector<pointer> new_ptrs;
        new_ptrs.reserve(size() - t_slice.num_step);
        for(auto i = 0; i < size(); ++i) {
            if(i < t_slice.l | i >= t_slice.r | (i - t_slice.l) % t_slice.step != 0) {
                new_ptrs.push_back(_ptrs[i]);
            }
        }
        _ptrs = std::move(new_ptrs);
    }

    size_type index(const_reference value) const {
        for(auto i = 0; i < size(); ++i) {
            if(*_ptrs[i] == value) { return i; }
        }   throw std::runtime_error("pyvec::index: value is not in the list");
    }

    size_type count(const_reference value) const {
        size_type ans = 0;
        for(const auto& ptr: _ptrs) {
            if(*ptr == value) { ++ans; }
        }   return ans;
    }

    void insert(difference_type pos, const_reference value) { insert(cbegin() + pos, value); }

    shared<T> pop() {
        auto ans = share(size() - 1);
        pop_back();
        return ans;
    }

    void reverse() {
        std::reverse(_ptrs.begin(), _ptrs.end());
    }

};

#endif // PY_VEC_HPP_
