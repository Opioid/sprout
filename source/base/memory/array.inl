#ifndef SU_BASE_MEMORY_ARRAY_INL
#define SU_BASE_MEMORY_ARRAY_INL

#include "align.hpp"
#include "array.hpp"

namespace memory {

template <typename T>
Array<T>::Array() noexcept : capacity_(0), size_(0), data_(nullptr) {}

template <typename T>
Array<T>::Array(uint64_t capacity, uint64_t size) noexcept
    : capacity_(capacity),
      size_(size > capacity ? capacity : size),
      data_(allocate_aligned<T>(capacity)) {}

template <typename T>
Array<T>::~Array() noexcept {
    free_aligned(data_);
}

template <typename T>
uint64_t Array<T>::size() const noexcept {
    return size_;
}

template <typename T>
T* Array<T>::data() noexcept {
    return data_;
}

template <typename T>
void Array<T>::reserve(uint64_t capacity) noexcept {
    size_ = 0;

    if (capacity_ >= capacity) {
        return;
    }

    free_aligned(data_);

    capacity_ = capacity;

    data_ = allocate_aligned<T>(capacity);
}

template <typename T>
void Array<T>::push_back(T const& v) noexcept {
    data_[size_] = v;

    ++size_;
}

template <typename T>
T& Array<T>::operator[](uint64_t i) noexcept {
    return data_[i];
}

}  // namespace memory

#endif
