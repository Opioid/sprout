#ifndef SU_CORE_IMAGE_TYPED_IMAGE_FWD_HPP
#define SU_CORE_IMAGE_TYPED_IMAGE_FWD_HPP

#include "base/math/vector.hpp"

namespace image {

struct Description;

class Image;

template <typename T>
class Typed_image;

template <typename T>
class Typed_sparse_image;

using Byte1 = Typed_image<uint8_t>;
using Byte2 = Typed_image<byte2>;
using Byte3 = Typed_image<byte3>;
using Byte4 = Typed_image<byte4>;

using Short3 = Typed_image<short3>;
using Short4 = Typed_image<short4>;

using Float1        = Typed_image<float>;
using Float1_sparse = Typed_sparse_image<float>;
using Float2        = Typed_image<float2>;
using Float3        = Typed_image<packed_float3>;
using Float4 = Typed_image<float4>;

}  // namespace image

#endif
