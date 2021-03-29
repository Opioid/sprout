#ifndef SU_CORE_IMAGE_CHANNELS_HPP
#define SU_CORE_IMAGE_CHANNELS_HPP

#include <cstdint>

namespace image {

enum class Swizzle { Undefined, X, Y, Z, W, XY, YX, XYZ, YXZ, XYZW, YXZW };

enum class Encoding { Color, Depth, SNorm, UNorm, UInt };

struct Layout {
    Encoding encoding;
    uint32_t num_channels;
};

}  // namespace image

#endif
