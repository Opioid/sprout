#ifndef SU_CORE_IMAGE_HELPER_HPP
#define SU_CORE_IMAGE_HELPER_HPP

#include "typed_image_fwd.hpp"

namespace image {

float3 average_and_max_3(Float4 const& image, float3& max);

float average_and_max_luminance(Float4 const& image, float& max);

}  // namespace image

#endif
