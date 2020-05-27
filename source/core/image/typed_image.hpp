#ifndef SU_CORE_IMAGE_TYPED_IMAGE_HPP
#define SU_CORE_IMAGE_TYPED_IMAGE_HPP

#include "base/math/vector3.hpp"
#include "typed_image_fwd.hpp"

namespace image {

struct Description {
    Description();
    Description(int2 dimensions, int32_t num_elements = 1);
    Description(int3 const& dimensions, int32_t num_elements, int3 const& offset);

    uint64_t num_pixels() const;

    int3 const& dimensions() const;

    int32_t area() const;

    int32_t volume() const;

    int32_t num_elements() const;

    int3 const& offset() const;

    int3 dimensions_;

    int32_t num_elements_;

    int3 offset_;
};

template <typename T>
class alignas(16) Typed_image {
  public:
    Typed_image();

    Typed_image(Description const& description);

    Typed_image(Typed_image&& other) noexcept;

    ~Typed_image();

    Description const& description() const;

    int2 coordinates_2(int32_t index) const;

    void resize(Description const& description);

    void clear(T v);

    T load(int32_t index) const;

    T* address(int32_t index) const;

    void store(int32_t index, T v);

    T const& at(int32_t index) const;

    T load(int32_t x, int32_t y) const;

    void store(int32_t x, int32_t y, T v);

    T load_element(int32_t x, int32_t y, int32_t element) const;

    T const& at(int32_t x, int32_t y) const;

    T const& at_element(int32_t x, int32_t y, int32_t element) const;

    T load(int32_t x, int32_t y, int32_t z) const;

    T const& at(int32_t x, int32_t y, int32_t z) const;

    T const& at_element(int32_t x, int32_t y, int32_t z, int32_t element) const;

    void gather(int4 const& xy_xy1, T c[4]) const;

    void square_transpose();

    T* data() const;

    void copy(Typed_image& destination) const;

  private:
    Description description_;

    T* data_ = nullptr;
};

template <typename T>
class Typed_sparse_image {
  public:
    Typed_sparse_image(Description const& description);

    Typed_sparse_image(Typed_sparse_image&& other) noexcept;

    ~Typed_sparse_image();

    Description const& description() const;

    T load(int64_t index) const;

    void store_sequentially(int64_t index, T v);

    T const& at(int64_t index) const;

    T load(int32_t x, int32_t y) const;

    void store(int32_t x, int32_t y, T v);

    T load_element(int32_t x, int32_t y, int32_t element) const;

    T const& at(int32_t x, int32_t y) const;

    T const& at_element(int32_t x, int32_t y, int32_t element) const;

    T load(int32_t x, int32_t y, int32_t z) const;

    T const& at(int32_t x, int32_t y, int32_t z) const;

    T const& at_element(int32_t x, int32_t y, int32_t z, int32_t element) const;

    void gather(int4 const& xy_xy1, T c[4]) const;

  private:
    int3 coordinates_3(int64_t index) const;

    static int32_t constexpr Log2_cell_dim = 4;
    static int32_t constexpr Cell_dim      = 1 << Log2_cell_dim;

    static T constexpr empty_ = T(0);

    Description description_;

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
extern template class Typed_image<ushort3>;
extern template class Typed_image<float>;
extern template class Typed_sparse_image<float>;
extern template class Typed_image<float2>;
extern template class Typed_image<packed_float3>;
extern template class Typed_image<float4>;

}  // namespace image

#endif
