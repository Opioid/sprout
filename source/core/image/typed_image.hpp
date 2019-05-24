#ifndef SU_CORE_IMAGE_TYPED_IMAGE_HPP
#define SU_CORE_IMAGE_TYPED_IMAGE_HPP

#include "base/math/vector.hpp"
#include "image.hpp"
#include "typed_image_fwd.hpp"

namespace image {

template <typename T>
class alignas(32) Typed_image final : public Image {
  public:
    Typed_image(Description const& description) noexcept;

    ~Typed_image() noexcept;

    Typed_image<T> clone() const noexcept;

    void resize(int2 dimensions, int32_t num_elements = 1) noexcept;
    void resize(int3 const& dimensions, int32_t num_elements = 1) noexcept;

    void clear(T v) noexcept;

    T load(int32_t index) const noexcept;

    T* address(int32_t index) const noexcept;

    void store(int32_t index, T v) noexcept;

    T const& at(int32_t index) const noexcept;

    T load(int32_t x, int32_t y) const noexcept;

    void store(int32_t x, int32_t y, T v) noexcept;

    T load_element(int32_t x, int32_t y, int32_t element) const noexcept;

    T const& at(int32_t x, int32_t y) const noexcept;

    T const& at_element(int32_t x, int32_t y, int32_t element) const noexcept;

    T load(int32_t x, int32_t y, int32_t z) const noexcept;

    T const& at(int32_t x, int32_t y, int32_t z) const noexcept;

    T const& at_element(int32_t x, int32_t y, int32_t z, int32_t element) const noexcept;

    void gather(int4 const& xy_xy1, T c[4]) const noexcept;

    void square_transpose() noexcept;

    T* data() const noexcept;

    size_t num_bytes() const noexcept;

  private:
    T* data_ = nullptr;
};

template <typename T>
class Typed_sparse_image final : public Image {
  public:
    Typed_sparse_image(Description const& description) noexcept;

    ~Typed_sparse_image() noexcept;

    T load(int64_t index) const noexcept;

    void store_sequentially(int64_t index, T v) noexcept;

    T const& at(int64_t index) const noexcept;

    T load(int32_t x, int32_t y) const noexcept;

    void store(int32_t x, int32_t y, T v) noexcept;

    T load_element(int32_t x, int32_t y, int32_t element) const noexcept;

    T const& at(int32_t x, int32_t y) const noexcept;

    T const& at_element(int32_t x, int32_t y, int32_t element) const noexcept;

    T load(int32_t x, int32_t y, int32_t z) const noexcept;

    T const& at(int32_t x, int32_t y, int32_t z) const noexcept;

    T const& at_element(int32_t x, int32_t y, int32_t z, int32_t element) const noexcept;

    void gather(int4 const& xy_xy1, T c[4]) const noexcept;

    size_t num_bytes() const noexcept;

  private:
    static int32_t constexpr Log2_cell_dim = 4;
    static int32_t constexpr Cell_dim      = 1 << Log2_cell_dim;

    static T constexpr empty_ = T(0);

    int3 num_cells_;

    struct Cell {
        T* data;
        T  value;
    };

    Cell* cells_ = nullptr;
};

extern template class Typed_image<uint8_t>;
extern template class Typed_image<byte2>;
extern template class Typed_image<byte3>;
extern template class Typed_image<byte4>;
extern template class Typed_image<float>;
extern template class Typed_sparse_image<float>;
extern template class Typed_image<float2>;
extern template class Typed_image<packed_float3>;
extern template class Typed_image<float4>;

}  // namespace image

#endif
