#ifndef SU_BASE_MEMORY_ARRAY_HPP
#define SU_BASE_MEMORY_ARRAY_HPP

#include <cstdint>
#include <initializer_list>

namespace memory {

template <typename T>
class Array {
  public:
    Array() noexcept;

    Array(uint64_t size) noexcept;

    Array(uint64_t size, T const& def) noexcept;

    Array(std::initializer_list<T> list) noexcept;

    Array(Array&& other) noexcept;

    ~Array() noexcept;

    bool empty() const noexcept;

    bool full() const noexcept;

    uint64_t capacity() const noexcept;
    uint64_t size() const noexcept;

    T const* data() const noexcept;
    T*       data() noexcept;

    void operator=(Array&& other) noexcept;

    void release() noexcept;

    void resize(uint64_t capacity) noexcept;

    void reserve(uint64_t capacity) noexcept;

    void push_back(T const& v) noexcept;

    T& operator[](uint64_t i) noexcept;

    T const* begin() const noexcept;
    T*       begin() noexcept;

    T const* end() const noexcept;
    T*       end() noexcept;

  private:
    void allocate(uint64_t capacity) noexcept;

    uint64_t capacity_;
    uint64_t size_;

    T* data_;
};

}  // namespace memory

#endif
