#ifndef SU_BASE_MEMORY_BUFFER_HPP
#define SU_BASE_MEMORY_BUFFER_HPP

#include <cstddef>

namespace memory {

template <typename T>
class Buffer {
  public:
    Buffer() : data_(nullptr) {}

    Buffer(size_t size) : data_(new T[size]) {}

    Buffer(size_t size, T const& def) : data_(new T[size]) {
        for (size_t i = 0; i < size; ++i) {
            data_[i] = def;
        }
    }

    Buffer(Buffer&& other) noexcept : data_(other.data_) {
        other.data_ = nullptr;
    }

    Buffer(Buffer const& other) : data_(other.data_) {
        // Not happy with this, but in practice it makes life easiser with std::function
        // On one hand I don't like the cast, on the other hand the uggliness is explicit now...
        const_cast<Buffer&>(other).data_ = nullptr;
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
