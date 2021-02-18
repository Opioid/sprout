#ifndef SU_BASE_MEMORY_ARRAY_HPP
#define SU_BASE_MEMORY_ARRAY_HPP

#include <atomic>
#include <cstdint>
#include <initializer_list>

namespace memory {

template <typename T>
class Array {
  public:
    Array();

    Array(uint32_t size);

    Array(uint32_t size, T const& def);

    Array(std::initializer_list<T> list);

    Array(Array&& other) noexcept;

    ~Array();

    bool empty() const;

    bool full() const;

    uint32_t capacity() const;

    uint32_t size() const;

    T const* data() const;
    T*       data();

    operator T const *() const;

    void operator=(Array&& other) noexcept;

    void clear();

    void release();

    void resize(uint32_t size);

    void reserve(uint32_t capacity);

    void push_back(T const& v);

    T const& operator[](uint32_t i) const;
    T&       operator[](uint32_t i);

    T const* begin() const;
    T*       begin();

    T const* end() const;
    T*       end();

  private:
    void allocate(uint32_t capacity);

    uint32_t capacity_;
    uint32_t size_;

    T* data_;
};

template <typename T, uint32_t N>
class Static_array {
  public:
    Static_array();

    bool empty() const;

    bool full() const;

    static uint32_t constexpr capacity();

    uint32_t size() const;

    T const* data() const;
    T*       data();

    void clear();

    void resize(uint32_t size);

    void push_back(T const& v);

    T const& operator[](uint32_t i) const;
    T&       operator[](uint32_t i);

    T const* begin() const;
    T*       begin();

    T const* end() const;
    T*       end();

  private:
    void allocate(uint32_t capacity);

    uint32_t size_;

    T data_[N];
};

template <typename T>
class Concurrent_array {
  public:
    Concurrent_array();

    Concurrent_array(uint32_t size);

    Concurrent_array(uint32_t size, T const& def);

    Concurrent_array(std::initializer_list<T> list);

    Concurrent_array(Concurrent_array&& other) noexcept;

    ~Concurrent_array();

    bool empty() const;

    bool full() const;

    uint32_t capacity() const;

    uint32_t size() const;

    T const* data() const;
    T*       data();

    void operator=(Concurrent_array&& other) noexcept;

    void clear();

    void release();

    void resize(uint32_t capacity);

    void reserve(uint32_t capacity);

    void push_back(T const& v);

    T const& operator[](uint32_t i) const;
    T&       operator[](uint32_t i);

    T const* begin() const;
    T*       begin();

    T const* end() const;
    T*       end();

  private:
    void allocate(uint32_t capacity);

    uint32_t capacity_;

    std::atomic<uint32_t> size_;

    T* data_;
};

}  // namespace memory

#endif
