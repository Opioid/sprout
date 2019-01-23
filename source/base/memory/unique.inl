#ifndef SU_BASE_MEMORY_UNIQUE_INL
#define SU_BASE_MEMORY_UNIQUE_INL

#include "unique.hpp"

namespace memory {

template <class T>
Unique_ptr<T>::Unique_ptr() noexcept : value_(nullptr) {}

template <class T>
Unique_ptr<T>::Unique_ptr(T* value) noexcept : value_(value) {}

template <class T>
Unique_ptr<T>::Unique_ptr(Unique_ptr&& other) noexcept {
    value_       = other.value_;
    other.value_ = nullptr;
}

template <class T>
Unique_ptr<T>::~Unique_ptr() noexcept {
    delete value_;
}

template <class T>
Unique_ptr<T>& Unique_ptr<T>::operator=(Unique_ptr other) noexcept {
    T* value     = value_;
    value_       = other.value_;
    other.value_ = value;

    return *this;
}

template <class T>
T& Unique_ptr<T>::operator*() noexcept {
    return *value_;
}

template <class T>
T const* Unique_ptr<T>::operator->() const noexcept {
    return value_;
}

template <class T>
T* Unique_ptr<T>::operator->() noexcept {
    return value_;
}

template <class T>
T* Unique_ptr<T>::release() noexcept {
    T* value = value_;

    value_ = nullptr;

    return value;
}

}  // namespace memory

#endif
