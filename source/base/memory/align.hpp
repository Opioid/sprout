#ifndef SU_BASE_MEMORY_ALIGN_HPP
#define SU_BASE_MEMORY_ALIGN_HPP

#include <cstddef>

namespace memory {

static size_t constexpr L1_cache_line_size = 64;

void* allocate_aligned(size_t size);

template <typename T>
T* allocate_aligned(size_t count) {
    return reinterpret_cast<T*>(allocate_aligned(count * sizeof(T)));
}

void free_aligned(void* pointer);

template <class T>
void destroy(T* t) {
    if (t) {
        t->~T();
    }
}

template <class T>
void destroy(T& t) {
    t.~T();
}

template <typename T, typename... P>
T* construct_aligned(size_t count, P&... ps) {
    // This is more complicated than expected. See for example:
    // https://stackoverflow.com/questions/8720425/array-placement-new-requires-unspecified-overhead-in-the-buffer
    // Basically there is a small memory overhead for placement new[] that is "unknown" beforehand,
    // but needs to be allocated as well.

    //	return new(allocate_aligned<T>(count)) T[count];

    // It is easier to construct the array elements individually.

    T* buffer = allocate_aligned<T>(count);

    for (size_t i = 0; i < count; ++i) {
        new (&buffer[i]) T(ps...);
    }

    return buffer;
}

template <typename T>
void destroy_aligned(T* objects, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        objects[i].~T();
    }

    free_aligned(objects);
}

template <typename T>
class Buffer {
  public:
    Buffer() : data_(nullptr) {}

    Buffer(size_t size) : data_(allocate_aligned<T>(size)) {}

    Buffer(Buffer&& other) noexcept : data_(other.data_) {
        other.data_ = nullptr;
    }

    // Not really happy with this, but in practice it makes life easiser with std::function
    Buffer(Buffer& other) : data_(other.data_) {
        other.data_ = nullptr;
    }

    ~Buffer() {
        free_aligned(data_);
    }

    void resize(size_t size) {
        free_aligned(data_);

        data_ = allocate_aligned<T>(size);
    }

    void release() {
        free_aligned(data_);

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

    operator T const*() const {
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
