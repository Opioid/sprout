#ifndef SU_CORE_IMAGE_TYPED_IMAGE_HPP
#define SU_CORE_IMAGE_TYPED_IMAGE_HPP

#include "base/math/vector.hpp"
#include "image.hpp"

namespace image {

template <typename T>
class Typed_image final : public Image {
  public:
    Typed_image() noexcept = default;
    Typed_image(const Image::Description& description) noexcept;

    ~Typed_image() noexcept;

    Typed_image<T> clone() const noexcept;

    void resize(const Image::Description& description) noexcept;

    void clear(T v) noexcept;

    T load(int32_t index) const noexcept;

    T* address(int32_t index) const noexcept;

    void store(int32_t index, T v) noexcept;

    T const& at(int32_t index) const noexcept;
    T&       at(int32_t index) noexcept;

    T load(int32_t x, int32_t y) const noexcept;

    void store(int32_t x, int32_t y, T v) noexcept;

    T load_element(int32_t x, int32_t y, int32_t element) const noexcept;

    T const& at(int32_t x, int32_t y) const noexcept;
    T&       at(int32_t x, int32_t y) noexcept;

    T const& at_element(int32_t x, int32_t y, int32_t element) const noexcept;
    T&       at_element(int32_t x, int32_t y, int32_t element) noexcept;

    T load(int32_t x, int32_t y, int32_t z) const noexcept;

    T const& at(int32_t x, int32_t y, int32_t z) const noexcept;
    T&       at(int32_t x, int32_t y, int32_t z) noexcept;

    T const& at_element(int32_t x, int32_t y, int32_t z, int32_t element) const noexcept;
    T&       at_element(int32_t x, int32_t y, int32_t z, int32_t element) noexcept;

    void gather(int4 const& xy_xy1, T c[4]) const noexcept;

    void square_transpose() noexcept;

    T* data() const noexcept;

    size_t num_bytes() const noexcept override final;

  private:
    T* data_ = nullptr;
};

}  // namespace image

#endif
