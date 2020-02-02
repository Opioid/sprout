#ifndef SU_BASE_MEMORY_ARRAY_INL
#define SU_BASE_MEMORY_ARRAY_INL

#include "align.hpp"
#include "array.hpp"

#include <utility>

namespace memory {

template <typename T>
Array<T>::Array() : capacity_(0), size_(0), data_(nullptr) {}

template <typename T>
Array<T>::Array(uint32_t size) : capacity_(size), size_(size), data_(allocate_aligned<T>(size)) {}

template <typename T>
Array<T>::Array(uint32_t size, T const& def) : Array<T>(size) {
    for (uint32_t i = 0; i < size; ++i) {
        data_[i] = def;
    }
}

template <typename T>
Array<T>::Array(std::initializer_list<T> list)
    : capacity_(uint32_t(list.size())), size_(0), data_(allocate_aligned<T>(list.size())) {
    for (auto e : list) {
        push_back(e);
    }
}

template <typename T>
Array<T>::Array(Array&& other)
    : capacity_(other.capacity_), size_(other.size_), data_(other.data_) {
    other.capacity_ = 0;
    other.size_     = 0;
    other.data_     = nullptr;
}

template <typename T>
Array<T>::~Array() {
    free_aligned(data_);
}

template <typename T>
bool Array<T>::empty() const {
    return 0 == size_;
}

template <typename T>
bool Array<T>::full() const {
    return capacity_ == size_;
}

template <typename T>
uint32_t Array<T>::capacity() const {
    return capacity_;
}

template <typename T>
uint32_t Array<T>::size() const {
    return size_;
}

template <typename T>
T const* Array<T>::data() const {
    return data_;
}

template <typename T>
T* Array<T>::data() {
    return data_;
}

template <typename T>
void Array<T>::operator=(Array&& other) {
    std::swap(size_, other.size_);
    std::swap(capacity_, other.capacity_);
    std::swap(data_, other.data_);
}

template <typename T>
void Array<T>::clear() {
    size_ = 0;
}

template <typename T>
void Array<T>::release() {
    free_aligned(data_);
    data_ = nullptr;

    size_     = 0;
    capacity_ = 0;
}

template <typename T>
void Array<T>::resize(uint32_t size) {
    size_ = size;

    allocate(size);
}

template <typename T>
void Array<T>::reserve(uint32_t capacity) {
    size_ = 0;

    allocate(capacity);
}

template <typename T>
void Array<T>::push_back(T const& v) {
    data_[size_] = v;

    ++size_;
}

template <typename T>
T const& Array<T>::operator[](uint32_t i) const {
    return data_[i];
}

template <typename T>
T& Array<T>::operator[](uint32_t i) {
    return data_[i];
}

template <typename T>
T const* Array<T>::begin() const {
    return &data_[0];
}

template <typename T>
T* Array<T>::begin() {
    return &data_[0];
}

template <typename T>
T const* Array<T>::end() const {
    return &data_[size_];
}

template <typename T>
T* Array<T>::end() {
    return &data_[size_];
}

template <typename T>
void Array<T>::allocate(uint32_t capacity) {
    if (capacity_ >= capacity) {
        return;
    }

    free_aligned(data_);

    capacity_ = capacity;

    data_ = allocate_aligned<T>(capacity);
}

template <typename T>
Concurrent_array<T>::Concurrent_array() : capacity_(0), size_(0), data_(nullptr) {}

template <typename T>
Concurrent_array<T>::Concurrent_array(uint32_t size)
    : capacity_(size), size_(size), data_(allocate_aligned<T>(size)) {}

template <typename T>
Concurrent_array<T>::Concurrent_array(uint32_t size, T const& def) : Array<T>(size) {
    for (uint32_t i = 0; i < size; ++i) {
        data_[i] = def;
    }
}

template <typename T>
Concurrent_array<T>::Concurrent_array(std::initializer_list<T> list)
    : capacity_(uint32_t(list.size())), size_(0), data_(allocate_aligned<T>(list.size())) {
    for (auto e : list) {
        push_back(e);
    }
}

template <typename T>
Concurrent_array<T>::Concurrent_array(Concurrent_array&& other)
    : capacity_(other.capacity_), size_(other.size_), data_(other.data_) {
    other.capacity_ = 0;
    other.size_     = 0;
    other.data_     = nullptr;
}

template <typename T>
Concurrent_array<T>::~Concurrent_array() {
    free_aligned(data_);
}

template <typename T>
bool Concurrent_array<T>::empty() const {
    return 0 == size_;
}

template <typename T>
bool Concurrent_array<T>::full() const {
    return capacity_ == size_;
}

template <typename T>
uint32_t Concurrent_array<T>::capacity() const {
    return capacity_;
}

template <typename T>
uint32_t Concurrent_array<T>::size() const {
    return size_;
}

template <typename T>
T const* Concurrent_array<T>::data() const {
    return data_;
}

template <typename T>
T* Concurrent_array<T>::data() {
    return data_;
}

template <typename T>
void Concurrent_array<T>::operator=(Concurrent_array&& other) {
    int32_t const tmp = size_;
    size_             = other.size_;
    other.size_       = tmp;

    std::swap(capacity_, other.capacity_);
    std::swap(data_, other.data_);
}

template <typename T>
void Concurrent_array<T>::clear() {
    size_ = 0;
}

template <typename T>
void Concurrent_array<T>::release() {
    free_aligned(data_);
    data_ = nullptr;

    size_     = 0;
    capacity_ = 0;
}

template <typename T>
void Concurrent_array<T>::resize(uint32_t size) {
    size_ = size;

    allocate(size);
}

template <typename T>
void Concurrent_array<T>::reserve(uint32_t capacity) {
    size_ = 0;

    allocate(capacity);
}

template <typename T>
void Concurrent_array<T>::push_back(T const& v) {
    uint32_t const current = size_.fetch_add(1, std::memory_order_relaxed);

    data_[current] = v;
}

template <typename T>
T const& Concurrent_array<T>::operator[](uint32_t i) const {
    return data_[i];
}

template <typename T>
T& Concurrent_array<T>::operator[](uint32_t i) {
    return data_[i];
}

template <typename T>
T const* Concurrent_array<T>::begin() const {
    return &data_[0];
}

template <typename T>
T* Concurrent_array<T>::begin() {
    return &data_[0];
}

template <typename T>
T const* Concurrent_array<T>::end() const {
    return &data_[size_];
}

template <typename T>
T* Concurrent_array<T>::end() {
    return &data_[size_];
}

template <typename T>
void Concurrent_array<T>::allocate(uint32_t capacity) {
    if (capacity_ >= capacity) {
        return;
    }

    free_aligned(data_);

    capacity_ = capacity;

    data_ = allocate_aligned<T>(capacity);
}

}  // namespace memory

#endif
