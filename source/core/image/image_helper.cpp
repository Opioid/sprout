#include "image_helper.hpp"
#include "typed_image.inl"

namespace image {

math::float3 average_3(const Image_float_4& image) {
	const auto& d = image.description();
	uint32_t len = d.dimensions.x * d.dimensions.y;

	float ilen = 1.f / static_cast<float>(len);

	math::float3 result = math::float3_identity;

	for (uint32_t i = 0; i < len; ++i) {
		result += ilen * image.at(i).xyz;
	}

	return result;
}

}
