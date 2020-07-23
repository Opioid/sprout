#ifndef SU_BASE_MEMORY_BUFFER_HPP
#define SU_BASE_MEMORY_BUFFER_HPP

#include <cstddef>

namespace memory {

template <typename T>
class Buffer {
  public:
    Buffer() : data_(nullptr) {}

    Buffer(size_t size) : data_(new T[size]) {}

    Buffer(Buffer&& other) noexcept : data_(other.data_) {
        other.data_ = nullptr;
    }

    // Not really happy with this, but in practice it makes life easiser with std::function
    Buffer(Buffer& other) : data_(other.data_) {
        other.data_ = nullptr;
    }

    ~Buffer() {
        delete[] data_;
    }

    void resize(size_t size) {
        delete[] data_;

        data_ = new T[size];
    }

    void release() {
        delete[] data_;

        data_ = nullptr;
    }

    operator bool() const {
        return data_ != nullptr;
    }

    T const* data() const {
        return data_;
    }

    T* data() {
        return data_;
    }

    operator T const *() const {
        return data_;
    }

    operator T*() {
        return data_;
    }

    template <typename I>
    T const& operator[](I i) const {
        return data_[i];
    }

    template <typename I>
    T& operator[](I i) {
        return data_[i];
    }

  private:
    T* data_;
};

}  // namespace memory

#endif
