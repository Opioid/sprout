#ifndef SU_CORE_IMAGE_CHANNELS_HPP
#define SU_CORE_IMAGE_CHANNELS_HPP

namespace image {

enum class Channels {
    None = 0,
    X    = 1 << 0,
    Y    = 1 << 1,
    Z    = 1 << 2,
    W    = 1 << 3,
    XY   = X | Y,
    XYZ  = XY | Z,
    XYZW = XYZ | W
};

enum class Swizzle { XYZW, YXZW };

enum class Encoding { Color, Depth, SNorm, UNorm, UInt };

}  // namespace image

#endif
