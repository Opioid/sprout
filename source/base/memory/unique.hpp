#ifndef SU_BASE_MEMORY_UNIQUE_HPP
#define SU_BASE_MEMORY_UNIQUE_HPP

namespace memory {

template <class T>
class Unique_ptr {
  public:
    Unique_ptr();

    explicit Unique_ptr(T* value);

    Unique_ptr(Unique_ptr&& other) noexcept;

    ~Unique_ptr();

    operator bool() const;

    bool operator!() const;

    Unique_ptr<T>& operator=(Unique_ptr other);

    T const& operator*() const;
    T&       operator*();

    T const* operator->() const;

    T* operator->();

    T* release();

  private:
    T* value_;
};

}  // namespace memory

#endif
