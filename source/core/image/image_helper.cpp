#include "image_helper.hpp"
#include "typed_image.inl"
#include "base/spectrum/rgb.inl"

namespace image {

math::float3 average_3(const Image_float_4& image) {
	uint32_t len = image.area();

	float ilen = 1.f / static_cast<float>(len);

	math::float3 average = math::float3_identity;

	for (uint32_t i = 0; i < len; ++i) {
		average += ilen * image.at(i).xyz;
	}

	return average;
}

float average_and_max_luminance(const Image_float_4& image, float& max) {
	uint32_t len = image.area();

	float ilen = 1.f / static_cast<float>(len);

	float average = 0.f;
	max = 0.f;

	for (uint32_t i = 0; i < len; ++i) {
		float luminance = spectrum::luminance(image.at(i).xyz);
		average += ilen * luminance;
		max = std::max(luminance, max);
	}

	return average;
}

}
