#pragma once

namespace image {

enum class Channels {
  None = 0,
  X = 1 << 0,
  Y = 1 << 1,
  Z = 1 << 2,
  W = 1 << 3,
  XY = X | Y,
  XYZ = XY | Z
};

enum class Swizzle { XYZW, YXZW };

}  // namespace image
