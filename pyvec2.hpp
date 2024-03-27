#include <vector>
#include <memory>
#include <stdexcept>

struct slice {
    ptrdiff_t start, stop, step;

    slice_type(
        const ptrdiff_t start, const ptrdiff_t stop, const ptrdiff_t step
    ) : start(start), stop(stop), step(step) {}
};

template<typename T>
class pyvec {
private:
    template<typename U>
    using vec = std::vector<U>;
    template<typename U>
    using shared = std::shared_ptr<U>;

    /*
     *  Private Data
     */
    
    shared<vec<vec<T>>> _resources;
    vec<pointer> _ptrs;

    size_type _chunk_pivot = 0;
    size_type _capacity = 0;

    vec<shared<vec<vec<T>>>> _shared_resources;
    
public:
    using value_type = T;
    using reference = T &;
    using const_reference = const T &;
    using pointer = T *;
    using const_pointer = const T *;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;

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

    void setitem(const slice_type& slice, const pyvec<T>& other);

    shared<T> getitem(difference_type index);

    pyvec<T> getitem(const slice_type& slice);

    void delitem(difference_type index);

    void delitem(const slice_type& slice);

    bool contains(const T& value) const;

    /*
     *  C++-Vector-Like Interface
     */

    // default constructor
    pyvec();

    // constructor with iterators
    template<typename InputIt>
    pyvec(InputIt first, InputItlast);

    // deep copy constructor
    pyvec(const pyvec<T>& other);
    pyvec(const vec<T>& other);

    // move constructor
    pyvec(pyvec<T>&& other);
    pyvec(vec<T>&& other);

    // initializer list constructor
    pyvec(std::initializer_list<T> il);

    // destructor
    ~pyvec();

    // operator =
    pyvec<T>& operator=(const pyvec<T>& other);
    pyvec<T>& operator=(pyvec<T>&& other);
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

    /*
     *  Vector-Like Capacity
     */

    bool empty() const;

    size_type size() const;

    void reserve(size_type new_cap);

    size_type capacity() const;

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
    void swap(pyvec<T>& other);

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

