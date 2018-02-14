#ifndef SU_CORE_IMAGE_IMGE_HELPER_HPP
#define SU_CORE_IMAGE_IMGE_HELPER_HPP

#include "typed_image_fwd.hpp"

namespace image {

float3 average_and_max_3(const Float4& image, float3& max);

float average_and_max_luminance(const Float4& image, float& max);

}

#endif
