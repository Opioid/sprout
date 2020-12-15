#ifndef SU_CORE_IMAGE_TYPED_IMAGE_FWD_HPP
#define SU_CORE_IMAGE_TYPED_IMAGE_FWD_HPP

#include "base/math/vector3.hpp"

namespace image {

struct Description {
    Description();
    Description(int2 dimensions, int32_t num_elements = 1);
    Description(int3_p dimensions, int32_t num_elements, int3_p offset);

    uint64_t num_pixels() const;

    int3_p dimensions() const;

    int32_t num_elements() const;

    int3 offset() const;

    int3 dimensions_;

    int32_t num_elements_;

    int3 offset_;
};

class Image;

template <typename T>
class Typed_image;

template <typename T>
class Typed_sparse_image;

using Byte1 = Typed_image<uint8_t>;
using Byte2 = Typed_image<byte2>;
using Byte3 = Typed_image<byte3>;
using Byte4 = Typed_image<byte4>;

using Short3 = Typed_image<ushort3>;

using Float1        = Typed_image<float>;
using Float1_sparse = Typed_sparse_image<float>;
using Float2        = Typed_image<float2>;
using Float3        = Typed_image<packed_float3>;
// using Float3 = Typed_image<float3>;
using Float4 = Typed_image<float4>;

}  // namespace image

#endif
