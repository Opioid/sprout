#ifndef SU_BASE_MEMORY_ARRAY_HPP
#define SU_BASE_MEMORY_ARRAY_HPP

#include <atomic>
#include <cstdint>
#include <initializer_list>

namespace memory {

template <typename T>
class Array {
  public:
    Array() noexcept;

    Array(uint32_t size) noexcept;

    Array(uint32_t size, T const& def) noexcept;

    Array(std::initializer_list<T> list) noexcept;

    Array(Array&& other) noexcept;

    ~Array() noexcept;

    bool empty() const noexcept;

    bool full() const noexcept;

    uint32_t capacity() const noexcept;

    uint32_t size() const noexcept;

    T const* data() const noexcept;
    T*       data() noexcept;

    void operator=(Array&& other) noexcept;

    void clear() noexcept;

    void release() noexcept;

    void resize(uint32_t size) noexcept;

    void reserve(uint32_t capacity) noexcept;

    void push_back(T const& v) noexcept;

    T const& operator[](uint32_t i) const noexcept;
    T&       operator[](uint32_t i) noexcept;

    T const* begin() const noexcept;
    T*       begin() noexcept;

    T const* end() const noexcept;
    T*       end() noexcept;

  private:
    void allocate(uint32_t capacity) noexcept;

    uint32_t capacity_;
    uint32_t size_;

    T* data_;
};

template <typename T>
class Concurrent_array {
  public:
    Concurrent_array() noexcept;

    Concurrent_array(uint32_t size) noexcept;

    Concurrent_array(uint32_t size, T const& def) noexcept;

    Concurrent_array(std::initializer_list<T> list) noexcept;

    Concurrent_array(Concurrent_array&& other) noexcept;

    ~Concurrent_array() noexcept;

    bool empty() const noexcept;

    bool full() const noexcept;

    uint32_t capacity() const noexcept;

    uint32_t size() const noexcept;

    T const* data() const noexcept;
    T*       data() noexcept;

    void operator=(Concurrent_array&& other) noexcept;

    void clear() noexcept;

    void release() noexcept;

    void resize(uint32_t capacity) noexcept;

    void reserve(uint32_t capacity) noexcept;

    void push_back(T const& v) noexcept;

    T const& operator[](uint32_t i) const noexcept;
    T&       operator[](uint32_t i) noexcept;

    T const* begin() const noexcept;
    T*       begin() noexcept;

    T const* end() const noexcept;
    T*       end() noexcept;

  private:
    void allocate(uint32_t capacity) noexcept;

    uint32_t capacity_;

    std::atomic<uint32_t> size_;

    T* data_;
};

}  // namespace memory

#endif
