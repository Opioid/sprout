#ifndef SU_BASE_MEMORY_ARRAY_HPP
#define SU_BASE_MEMORY_ARRAY_HPP

#include <cstdint>

namespace memory {

template <typename T>
class Array {
  public:
    Array() noexcept;

    Array(uint64_t capacity, uint64_t size = ~0x0) noexcept;

    ~Array() noexcept;

    uint64_t size() const noexcept;

    T* data() noexcept;

    void reserve(uint64_t capacity) noexcept;

    void push_back(T const& v) noexcept;

    T& operator[](uint64_t i) noexcept;

  private:
    uint64_t size_;
    uint64_t capacity_;

    T* data_;
};

}  // namespace memory

#endif
