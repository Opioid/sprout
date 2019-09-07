#ifndef SU_BASE_MEMORY_ALIGN_HPP
#define SU_BASE_MEMORY_ALIGN_HPP

#include <cstddef>

namespace memory {

static size_t constexpr L1_cache_line_size = 64;

void* allocate_aligned(size_t size) noexcept;

template <typename T>
T* allocate_aligned(size_t count) noexcept {
    return reinterpret_cast<T*>(allocate_aligned(count * sizeof(T)));
}

void free_aligned(void* pointer) noexcept;

template <class T>
void destroy(T* t) noexcept {
    if (t) {
        t->~T();
    }
}

template <class T>
void destroy(T& t) noexcept {
    t.~T();
}

template <typename T, typename... P>
T* construct_array_aligned(size_t count, P&... ps) noexcept {
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
void destroy_aligned(T* objects, size_t count) noexcept {
    for (size_t i = 0; i < count; ++i) {
        objects[i].~T();
    }

    free_aligned(objects);
}

template <typename T>
class Buffer {
  public:
    Buffer(size_t size) noexcept : data_(allocate_aligned<T>(size)) {}

    ~Buffer() noexcept {
        free_aligned(data_);
    }

    T* data() noexcept {
        return data_;
    }

    operator T*() noexcept {
        return data_;
    }

    template <class I>
    T const& operator[](I i) const noexcept {
        return data_[i];
    }

    template <class I>
    T& operator[](I i) noexcept {
        return data_[i];
    }

  private:
    T* data_;
};

}  // namespace memory

#endif
