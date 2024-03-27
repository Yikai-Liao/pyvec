#include <vector>
#include <memory>
#include <stdexcept>


struct slice {
    ptrdiff_t start, stop, step;

    slice(
        const ptrdiff_t start, const ptrdiff_t stop, const ptrdiff_t step
    ) : start(start), stop(stop), step(step) {}
};

template<typename T>
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
    template<typename U>
    using vec = std::vector<U>;
    template<typename U>
    using shared = std::shared_ptr<U>;

    /*
     *  Private Data
     */
    constexpr size_t min_chunk_size = 64;

    shared<vec<vec<T>>> _resources;
    vec<pointer> _ptrs;

    size_type _chunk_pivot = 0;
    size_type _capacity = 0;

    vec<shared<vec<vec<T>>>> _shared_resources;

public:
    /*
     *  Iterator Declaration
     */

    using pointer_iterator = typename std::vector<T>::iterator;
    class const_iterator;
    class iterator;

    /*
     *  Pyhton-List-Like Interface
     */

    void append(const T& value);
    void append(T&& value);

    size_type count(const T& value) const;

    void extend(const pyvec<T>& other);
    void extend(pyvec<T>&& other);

    void insert(difference_type index, const T& value);

    shared<T> pop(difference_type index = -1);

    void remove(const T& value);

    void reverse();

    void clear();

    pyvec<T> copy();

    pyvec<T> deepcopy();

    void sort(bool reverse = false);
    template<typename Key>
    void sort(bool reverse, Key key);

    bool is_sorted(bool reverse = false) const;
    template<typename Key>
    bool is_sorted(bool reverse, Key key) const;

    template<typename Func>
    void filter(Func func);

    size_t index(const T& value, const difference_type start = 0) const;
    size_t index(const T& value, const difference_type start, const difference_type stop) const;

    /*
     *  Python Magic Method
     *  __getitem__, __setitem__, __delitem__, __contains__
     */

    void setitem(difference_type index, const T& value);

    void setitem(const slice& t_slice, const pyvec<T>& other);

    shared<T> getitem(difference_type index);

    pyvec<T> getitem(const slice& t_slice);

    void delitem(difference_type index);

    void delitem(const slice& t_slice);

    bool contains(const T& value) const;

    /*
     *  C++-Vector-Like Interface
     */

    // default constructor
    pyvec();

    // constructor with iterators
    template<typename InputIt>
    pyvec(InputIt first, InputIt last);

    // deep copy constructor
    pyvec(const pyvec<T>& other);
    explicit pyvec(const vec<T>& other);

    // move constructor
    pyvec(pyvec<T>&& other) noexcept ;
    explicit pyvec(vec<T>&& other);

    // initializer list constructor
    pyvec(std::initializer_list<T> il);

    // destructor
    ~pyvec() = default;

    // operator =
    pyvec<T>& operator=(const pyvec<T>& other);
    pyvec<T>& operator=(pyvec<T>&& other) noexcept ;
    pyvec<T>& operator=(std::initializer_list<T> il);

    // assign
    void assign(size_type count, const T& value);

    template<class InputIt>
    void assign(InputIt first, InputIt last);

    void assign(std::initializer_list<T> il);

    /*
     *  Vector-Like Element Access
     */
    
    reference at(size_type pos);
    const_reference at(size_type pos) const;

    reference operator[](size_type pos);
    const_reference operator[](size_type pos) const;

    reference front();
    const_reference front() const;

    reference back();
    const_reference back() const;

    // data() is removed since pyvec is not contiguous

    /*
     *  Vector-Like Iterators
     */

    iterator begin();
    const_iterator begin() const;
    const_iterator cbegin() const;

    iterator end();
    const_iterator end() const;
    const_iterator cend() const;

    pointer_iterator pbegin();

    pointer_iterator pend();

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

    // insert
    iterator insert(const_iterator pos, const T& value);
    iterator insert(const_iterator pos, T&& value);
    iterator insert(const_iterator pos, size_type count, const T& value);

    template<class InputIt>
    iterator insert(const_iterator pos, InputIt first, InputIt last);

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

private:
    /*
     *  Internal Helper Functions
     */
    void move_assign(pyvec<T>&& other);
    void move_assign(vec<T>&& other);

    void try_init();

    vec<T>& new_chunck(size_type n);

    vec<T>& add_chunck(vec<T> && chunck);

    [[nodiscard]] vec<T>& suitable_chunck(size_type expected_size);

    size_type insert_empty(const_iterator pos, size_type count);
};

// Iterator Definition
template<typename T>
class pyvec<T>::iterator {
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

template<typename T>
class pyvec<T>::const_iterator {
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
 *  Helper Functions
 */

template<typename T>
void pyvec<T>::move_assign(pyvec<T>&& other) {
    _resources = std::move(other._resources);
    _ptrs = std::move(other._ptrs);
    _chunk_pivot = other._chunk_pivot;
    _capacity = other._capacity;
    _shared_resources = std::move(other._shared_resources);
}

template<typename T>
void pyvec<T>::move_assign(vec<T>&& other) {
    clear();
    auto & chunck = add_chunck(std::move(other));
    _ptrs.reserve(chunck.size());
    for(auto & item : chunck) {
        _ptrs.push_back(&item);
    }
}

template<typename T>
void pyvec<T>::try_init() {
    if (!_resources) {
        _resources = std::make_shared<vec<vec<T>>>();
    }
}

template<typename T>
std::vector<T>& pyvec<T>::new_chunck(size_type n) {
    _resources->emplace_back();
    auto& chunck = _resources->back();
    chunck.reserve(n);
    _capacity += n;
    return chunck;
}

template<typename T>
std::vector<T>& pyvec<T>::add_chunck(vec<T>&& chunck) {
    _resources->emplace_back(std::move(chunck));
    _capacity += chunck.capacity();
    return _resources->back();
}

template<typename T>
std::vector<T>& pyvec<T>::suitable_chunck(size_type expected_size) {
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

template<typename T>
size_t pyvec<T>::insert_empty(const const_iterator pos, const size_type count) {
    const auto idx = std::distance(_ptrs.data(), pos._ptr);
    if (idx > _ptrs.size()) { throw std::out_of_range("pyvec::insert_empty"); }
    _ptrs.resize(_ptrs.size() + count);
    for (auto i = _ptrs.size() - 1; i >= idx + count; --i) {
        _ptrs[i] = _ptrs[i - count];
    }
    return idx;
}

/*
 *  Constructor
 */

template<typename T>
pyvec<T>::pyvec() { assign({}); }

template<typename T> template<typename InputIt>
pyvec<T>::pyvec(InputIt first, InputIt last) { assign(first, last); }

template<typename T>
pyvec<T>::pyvec(const std::vector<T>& other) {
    assign(other.begin(), other.end());
}

template<typename T>
pyvec<T>::pyvec(const pyvec<T>& other) {
    assign(other.begin(), other.end());
}

template<typename T>
pyvec<T>::pyvec(std::initializer_list<T> il) { assign(il); }

template<typename T>
pyvec<T>::pyvec(pyvec<T>&& other) noexcept {
    move_assign(other);
}

template<typename T>
pyvec<T>::pyvec(std::vector<T>&& other) {
    move_assign(other);
}

/*
 *  Operator =
 */

template<typename T>
pyvec<T>& pyvec<T>::operator=(const pyvec<T>& other) {
    assign(other.begin(), other.end());
    return *this;
}

template<typename T>
pyvec<T>& pyvec<T>::operator=(std::initializer_list<T> il) {
    assign(il);
    return *this;
}

template<typename T>
pyvec<T>& pyvec<T>::operator=(pyvec<T>&& other)  noexcept {
    move_assign(other);
    return *this;
}

/*
 *  Assign
 */

template<typename T>
void pyvec<T>::assign(size_type count, const T& value) {
    clear();
    auto & chunck  = suitable_chunck(count);
    chunck.assign(count, value);
    _ptrs.reserve(count);
    for(auto & item : chunck) {
        _ptrs.push_back(&item);
    }
}

template<typename T> template<class InputIt>
void pyvec<T>::assign(InputIt first, InputIt last) {
    clear();
    auto tmp_chunck = vec<T>(first, last);
    auto & chunck = add_chunck(std::move(tmp_chunck));
    _ptrs.reserve(chunck.size());
    for(auto & item : chunck) {
        _ptrs.push_back(&item);
    }
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
    if(pos >= size()) {
        throw std::out_of_range("pyvec::at");
    }   return *(_ptrs[pos]);
}

template<typename T>
T& pyvec<T>::at(size_t pos) {
    if(pos >= size()) {
        throw std::out_of_range("pyvec::at");
    }   return *(_ptrs[pos]);
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
typename pyvec<T>::pointer_iterator pyvec<T>::pbegin() {
    return _ptrs.data();
}

template<typename T>
typename pyvec<T>::pointer_iterator pyvec<T>::pend() {
    return _ptrs.data() + _ptrs.size();
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
    if (const auto delta = new_cap - _capacity; delta > 0) {
        _ptrs.reserve(std::max(new_cap, _ptrs.size() + delta));
        new_chunck(std::max(min_chunk_size, delta));
    }
}

template<typename T>
size_t pyvec<T>::capacity() const {
    return _capacity;
}

template<typename T>
void pyvec<T>::shrink_to_fit() {
    try_init();
    _ptrs.shrink_to_fit();
    _capacity = 0;
    for(auto & chunk: *_resources) {
        chunk.shrink_to_fit();
        _capacity += chunk.capacity();
    }
}

/*
 *  Vector-Like Modifiers
 */

template<typename T>
void pyvec<T>::clear() {
    _resources = std::make_shared<vec<vec<T>>>();
    _ptrs.clear();
    _shared_resources.clear();
    _chunk_pivot = _capacity = 0;
}

template<typename T>
auto pyvec<T>::insert(const const_iterator pos, const T& value) {
    const auto idx = insert_empty(pos, 1);
    auto & chunk = suitable_chunck(1);
    chunk.push_back(value);
    _ptrs[idx] = &chunk.back();
    return iterator(_ptrs.data() + idx);
}

template<typename T>
auto pyvec<T>::insert(const const_iterator pos, T&& value) {
    const auto idx = insert_empty(pos, 1);
    auto & chunk = suitable_chunck(1);
    chunk.push_back(value);
    _ptrs[idx] = &chunk.back();
    return iterator(_ptrs.data() + idx);
}

template<typename T>
auto pyvec<T>::insert(const_iterator pos, const size_type count, const T& value) {
    const auto idx = insert_empty(pos, count);
    auto chunk = suitable_chunck(count);
    // insert the new elements
    for(auto i = idx; i < idx + count; ++i) {
        chunk.push_back(value);
        _ptrs[i] = &chunk.back();
    }   return iterator(_ptrs.data() + idx);
}

template<typename T>
auto pyvec<T>::insert(const const_iterator pos, std::initializer_list<T> il) {
    return insert(pos, il.begin(), il.end());
}

template<typename T> template<class InputIt>
typename pyvec<T>::iterator pyvec<T>::insert(const const_iterator pos, InputIt first, InputIt last) {
    const auto count = std::distance(first, last);
    const auto idx = insert_empty(pos, count);
    auto chunk = suitable_chunck(count);
    // insert the new elements
    for(auto it = first; it != last; ++it) {
        chunk.push_back(*it);
        _ptrs[idx++] = &chunk.back();
    }   return iterator(_ptrs.data() + idx);
}

template<typename T> template<class... Args>
typename pyvec<T>::iterator pyvec<T>::emplace(const const_iterator pos, Args&&... args) {
    const auto idx = insert_empty(pos, 1);
    auto & chunk = suitable_chunck(1);
    chunk.emplace_back(std::forward<Args>(args)...);
    _ptrs[idx] = &chunk.back();
    return iterator(_ptrs.data() + idx);
}

template<typename T>
auto pyvec<T>::erase(const_iterator pos) {
    const auto idx = std::distance(_ptrs.data(), pos._ptr);
    if(idx >= _ptrs.size() | idx < 0) {
        throw std::out_of_range("pyvec::erase");
    }
    _ptrs.erase(_ptrs.begin() + idx);
    return iterator(_ptrs.data() + idx);
}

template<typename T>
auto pyvec<T>::erase(const_iterator first, const_iterator last){
    const auto left = std::distance(_ptrs.data(), first._ptr);
    const auto right = std::distance(_ptrs.data(), last._ptr);
    if(left >= _ptrs.size() | left < 0 | right > _ptrs.size() | right < 0) {
        throw std::out_of_range("pyvec::erase");
    }
    _ptrs.erase(_ptrs.begin() + left, _ptrs.begin() + right);
    return iterator(_ptrs.data() + left);
}

template<typename T>
void pyvec<T>::push_back(const T& value) {
    auto & chunk = suitable_chunck(1);
    chunk.push_back(value);
    _ptrs.push_back(&chunk.back());
}

template<typename T>
void pyvec<T>::push_back(T&& value) {
    auto & chunk = suitable_chunck(1);
    chunk.push_back(value);
    _ptrs.push_back(&chunk.back());
}

template<typename T> template<class... Args>
typename pyvec<T>::reference pyvec<T>::emplace_back(Args&&... args) {
    auto & chunk = suitable_chunck(1);
    chunk.emplace_back(std::forward<Args>(args)...);
    _ptrs.push_back(&chunk.back());
    return chunk.back();
}

template<typename T>
void pyvec<T>::pop_back() {
    if(_ptrs.empty()) {
        throw std::out_of_range("pyvec::pop_back");
    }   _ptrs.pop_back();
}

template<typename T>
void pyvec<T>::resize(size_type count) {
    if(count <= size()) {
        return _ptrs.resize(count);
    }
    const auto delta = count - size();
    auto & chunk = suitable_chunck(delta);
    auto idx = chunk.size();
    chunk.resize(idx + delta);
    _ptrs.reserve(count);
    for(; idx<chunk.size(); ++idx) {
        _ptrs.push_back(&chunk[idx]);
    }
}

template<typename T>
void pyvec<T>::resize(size_type count, const T& value) {
    if(count <= size()) {
        return _ptrs.resize(count);
    }
    const auto delta = count - size();
    auto & chunk = suitable_chunck(delta);
    auto idx = chunk.size();
    chunk.resize(idx + delta, value);
    _ptrs.reserve(count);
    for(; idx<chunk.size(); ++idx) {
        _ptrs.push_back(&chunk[idx]);
    }
}

template<typename T>
void pyvec<T>::swap(pyvec<T>& other) noexcept {
    std::swap(_resources, other._resources);
    std::swap(_ptrs, other._ptrs);
    std::swap(_chunk_pivot, other._chunk_pivot);
    std::swap(_capacity, other._capacity);
    std::swap(_shared_resources, other._shared_resources);
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
    for (auto it = cbegin(), it2 = other.cbegin(); it != cend(); ++it, ++it2) {
        if (*it < *it2) { return true; }
        if (*it > *it2) { return false; }
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
void pyvec<T>::append(const T& value) { push_back(value); }

template<typename T>
void pyvec<T>::append(T&& value) { push_back(std::move(value)); }

template<typename T>
size_t pyvec<T>::count(const T& value) const {
    size_t cnt = 0;
    for(auto & item: _ptrs) {
        if(*item == value) ++cnt;
    }   return cnt;
}

template<typename T>
void pyvec<T>::extend(const pyvec<T>& other) {
    _shared_resources.push_back(other._resources);
    _ptrs.reserve(_ptrs.size() + other.size());
    for(auto & ptr: other._ptrs) {
        _ptrs.push_back(ptr);
    }
}

template<typename T>
void pyvec<T>::extend(pyvec<T>&& other) {
    _shared_resources.push_back(std::move(other._resources));
    _ptrs.reserve(_ptrs.size() + other.size());
    for(auto & ptr: other._ptrs) {
        _ptrs.push_back(ptr);
    }
}

template<typename T>
pyvec<T> pyvec<T>::copy() {
    pyvec<T> ans{};
    ans.extend(*this);
    return ans;
}

template<typename T>
pyvec<T> pyvec<T>::deepcopy() {
    return pyvec(*this);
}