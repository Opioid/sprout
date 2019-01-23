#ifndef SU_BASE_MEMORY_UNIQUE_HPP
#define SU_BASE_MEMORY_UNIQUE_HPP

namespace memory {

template <class T>
class Unique_ptr {
  public:
    Unique_ptr() noexcept;

    explicit Unique_ptr(T* value) noexcept;

    Unique_ptr(Unique_ptr&& other) noexcept;

    ~Unique_ptr() noexcept;

    Unique_ptr<T>& operator=(Unique_ptr other) noexcept;

    T& operator*() noexcept;

    T const* operator->() const noexcept;

    T* operator->() noexcept;

    T* release() noexcept;

  private:
    T* value_;
};

}  // namespace memory

#endif
