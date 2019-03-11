#ifndef SU_BASE_MEMORY_ARRAY_INL
#define SU_BASE_MEMORY_ARRAY_INL

#include <utility>
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
Array<T>::Array(std::initializer_list<T> list) noexcept
    : capacity_(list.size()), size_(0), data_(allocate_aligned<T>(list.size())) {
    for (auto e : list) {
        push_back(e);
    }
}

template <typename T>
Array<T>::Array(Array&& other) noexcept
    : capacity_(other.capacity_), size_(other.size_), data_(other.data_) {
    other.capacity_ = 0;
    other.size_     = 0;
    other.data_     = nullptr;
}

template <typename T>
Array<T>::~Array() noexcept {
    free_aligned(data_);
}

template <typename T>
bool Array<T>::empty() const noexcept {
    return 0 == size_;
}

template <typename T>
bool Array<T>::full() const noexcept {
    return capacity_ == size_;
}

template <typename T>
uint64_t Array<T>::size() const noexcept {
    return size_;
}

template <typename T>
T const* Array<T>::data() const noexcept {
    return data_;
}

template <typename T>
T* Array<T>::data() noexcept {
    return data_;
}

template <typename T>
void Array<T>::operator=(Array&& other) noexcept {
    std::swap(size_, other.size_);
    std::swap(capacity_, other.capacity_);
    std::swap(data_, other.data_);
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

template <typename T>
T const* Array<T>::begin() const noexcept {
    return &data_[0];
}

template <typename T>
T* Array<T>::begin() noexcept {
    return &data_[0];
}

template <typename T>
T const* Array<T>::end() const noexcept {
    return &data_[size_];
}

template <typename T>
T* Array<T>::end() noexcept {
    return &data_[size_];
}

}  // namespace memory

#endif
