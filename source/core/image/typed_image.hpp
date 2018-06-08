#ifndef SU_CORE_IMAGE_TYPED_IMAGE_HPP
#define SU_CORE_IMAGE_TYPED_IMAGE_HPP

#include "base/math/vector.hpp"
#include "image.hpp"

namespace image {

template <typename T>
class Typed_image final : public Image {
  public:
    Typed_image() = default;
    Typed_image(const Image::Description& description);
    ~Typed_image();

    Typed_image<T> clone() const;

    void resize(const Image::Description& description);

    void clear(T v);

    T load(int32_t index) const;

    T* address(int32_t index) const;

    void store(int32_t index, T v);

    T const& at(int32_t index) const;
    T&       at(int32_t index);

    T load(int32_t x, int32_t y) const;

    void store(int32_t x, int32_t y, T v);

    T load_element(int32_t x, int32_t y, int32_t element) const;

    T const& at(int32_t x, int32_t y) const;
    T&       at(int32_t x, int32_t y);

    T const& at_element(int32_t x, int32_t y, int32_t element) const;
    T&       at_element(int32_t x, int32_t y, int32_t element);

    T load(int32_t x, int32_t y, int32_t z) const;

    T const& at(int32_t x, int32_t y, int32_t z) const;
    T&       at(int32_t x, int32_t y, int32_t z);

    T const& at_element(int32_t x, int32_t y, int32_t z, int32_t element) const;
    T&       at_element(int32_t x, int32_t y, int32_t z, int32_t element);

    void gather(int4 const& xy_xy1, T c[4]) const;

    void square_transpose();

    T* data() const;

    virtual size_t num_bytes() const override final;

  private:
    T* data_ = nullptr;
};

}  // namespace image

#endif
