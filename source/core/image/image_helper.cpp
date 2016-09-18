#include "image_helper.hpp"
#include "typed_image.inl"
#include "base/spectrum/rgb.inl"

namespace image {

float3 average_and_max_3(const Image_float_4& image, float3& max) {
	uint32_t len = image.area();

	float ilen = 1.f / static_cast<float>(len);

	float3 average = math::float3_identity;
	max = math::float3_identity;

	for (uint32_t i = 0; i < len; ++i) {
		const float4& pixel = image.at(i);
		average += ilen * pixel.xyz;
		max = float3(std::max(pixel.x, max.x),
					 std::max(pixel.y, max.y),
					 std::max(pixel.z, max.z));
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
