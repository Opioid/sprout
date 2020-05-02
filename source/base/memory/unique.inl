#ifndef SU_BASE_MEMORY_UNIQUE_INL
#define SU_BASE_MEMORY_UNIQUE_INL

#include "unique.hpp"

namespace memory {

template <class T>
Unique_ptr<T>::Unique_ptr() : value_(nullptr) {}

template <class T>
Unique_ptr<T>::Unique_ptr(T* value) : value_(value) {}

template <class T>
Unique_ptr<T>::Unique_ptr(Unique_ptr&& other) noexcept : value_(other.value_) {
    other.value_ = nullptr;
}

template <class T>
Unique_ptr<T>::Unique_ptr(Unique_ptr& other) : value_(other.value_) {
    other.value_ = nullptr;
}

template <class T>
Unique_ptr<T>::~Unique_ptr() {
    delete value_;
}

template <class T>
Unique_ptr<T>::operator bool() const {
    return nullptr != value_;
}

template <class T>
bool Unique_ptr<T>::operator!() const {
    return nullptr == value_;
}

template <class T>
Unique_ptr<T>& Unique_ptr<T>::operator=(Unique_ptr other) {
    T* value     = value_;
    value_       = other.value_;
    other.value_ = value;

    return *this;
}

template <class T>
T const& Unique_ptr<T>::operator*() const {
    return *value_;
}

template <class T>
T& Unique_ptr<T>::operator*() {
    return *value_;
}

template <class T>
T const* Unique_ptr<T>::operator->() const {
    return value_;
}

template <class T>
T* Unique_ptr<T>::operator->() {
    return value_;
}

template <class T>
T* Unique_ptr<T>::release() {
    T* value = value_;

    value_ = nullptr;

    return value;
}

}  // namespace memory

#endif
