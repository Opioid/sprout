#ifndef SU_CORE_IMAGE_TYPED_IMAGE_HPP
#define SU_CORE_IMAGE_TYPED_IMAGE_HPP

#include "base/math/vector3.hpp"
#include "typed_image_fwd.hpp"

namespace image {

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

    void store(int32_t index, T v);

    T at(int32_t index) const;

    void store(int32_t x, int32_t y, T v);

    T at(int32_t x, int32_t y) const;

    void gather(int4_p xy_xy1, T c[4]) const;

    T at(int32_t x, int32_t y, int32_t z) const;

    void gather(int3_p xyz, int3_p xyz1, T c[8]) const;

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

    void store_sequentially(int64_t index, T v);

    T at(int64_t index) const;

    void store(int32_t x, int32_t y, T v);

    T at(int32_t x, int32_t y) const;

    T at_element(int32_t x, int32_t y, int32_t element) const;

    T at(int32_t x, int32_t y, int32_t z) const;

    void gather(int4_p xy_xy1, T c[4]) const;

    void gather(int3_p xyz, int3_p xyz1, T c[8]) const;

  private:
    int3 coordinates_3(int64_t index) const;

    static int32_t constexpr Log2_cell_dim = 4;
    static int32_t constexpr Cell_dim      = 1 << Log2_cell_dim;

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
