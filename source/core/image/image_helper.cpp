#include "image_helper.hpp"
#include "typed_image.inl"
#include "base/math/vector4.inl"
#include "base/spectrum/rgb.hpp"

namespace image {

float3 average_and_max_3(const Float_4& image, float3& max) {
	const int32_t len = image.volume();

	const float ilen = 1.f / static_cast<float>(len);

	float3 average(0.f);
	max = float3(0.f);

	for (int32_t i = 0; i < len; ++i) {
		const float4& pixel = image.at(i);
		float3 pixel3 = pixel.xyz();
		average += ilen * pixel3;
		max = float3(std::max(pixel3[0], max[0]),
					 std::max(pixel3[1], max[1]),
					 std::max(pixel3[2], max[2]));
	}

	return average;
}

float average_and_max_luminance(const Float_4& image, float& max) {
	const int32_t len = image.volume();

	const float ilen = 1.f / static_cast<float>(len);

	float average = 0.f;
	max = 0.f;

	for (int32_t i = 0; i < len; ++i) {
		float luminance = spectrum::luminance(image.at(i).xyz());
		average += ilen * luminance;
		max = std::max(luminance, max);
	}

	return average;
}

}
