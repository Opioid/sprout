#ifndef SU_CORE_IMAGE_TEXTURE_TYPED_TEXTURE_HPP
#define SU_CORE_IMAGE_TEXTURE_TYPED_TEXTURE_HPP

#include <cstddef>
#include "image/typed_image_fwd.hpp"
#include "base/math/vector.hpp"

namespace image::texture {

template <typename T>
class Typed_texture {
public:
    Typed_texture(T const& image);

    Image const& image() const noexcept;

    int32_t num_channels() const noexcept;
    int32_t num_elements() const noexcept;

    int2        dimensions_2() const noexcept;
    int3 const& dimensions_3() const noexcept;

    size_t image_num_bytes() const noexcept;

protected:
    T const& image_;
};

extern template class Typed_texture<Byte1>;
extern template class Typed_texture<Byte2>;
extern template class Typed_texture<Byte3>;
extern template class Typed_texture<Byte4>;
extern template class Typed_texture<Float1>;
extern template class Typed_texture<Float1_sparse>;
extern template class Typed_texture<Float2>;
extern template class Typed_texture<Float3>;

}

#endif
