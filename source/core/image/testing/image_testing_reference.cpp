#include "image_testing_reference.hpp"
#include "base/color/color.inl"
#include "miniz/miniz.hpp"
#include <fstream>

namespace image { namespace testing {

// the normal map will look correct if tangent follows same direction as texture coordinates
// positive z is pointing into the screen

// [0, 0]-----[1, 0]
// |			   |
// |			   |
// |			   |
// |			   |
// |			   |
// [0, 1]-----[1, 1]

// tangent		[1,  0,  0]
// bi-tangent	[0, -1,  0]
// normal       [0,  0, -1]

math::float3 reference_normal(math::float2 p, math::float2 range) {
	float vx = -1.f + p.x * range.x;
	float vy = -1.f + p.y * range.y;

	math::float2 xy(vx, vy);
	float l = math::length(xy);

	math::float3 v;

	const float radius = 1.f - 0.5f * range.y;
	if (l < radius) {
		v = math::float3(vx, vy, 1.f - l);
	} else {
		v = math::float3(0.f, 0.f, 1.f);
	}

	return math::normalized(v);
}

void create_reference_normal_map(math::int2 dimensions) {
	color::Color4c* rgba = new color::Color4c[dimensions.x * dimensions.y];

	math::float2 range(2.f / static_cast<float>(dimensions.x),
					   2.f / static_cast<float>(dimensions.y));

	for (int32_t y = 0; y < dimensions.y; ++y) {
		for (int32_t x = 0; x < dimensions.x; ++x) {
			auto& pixel = rgba[y * dimensions.x + x];

			float fx = static_cast<float>(x);
			float fy = static_cast<float>(y);

			math::float3 v0  = reference_normal(math::float2(fx + 0.125f, fy + 0.125f), range);
			math::float3 v1  = reference_normal(math::float2(fx + 0.375f, fy + 0.125f), range);
			math::float3 v2  = reference_normal(math::float2(fx + 0.75f,  fy + 0.125f), range);
			math::float3 v3  = reference_normal(math::float2(fx + 0.875f, fy + 0.125f), range);

			math::float3 v4  = reference_normal(math::float2(fx + 0.125f, fy + 0.375f), range);
			math::float3 v5  = reference_normal(math::float2(fx + 0.375f, fy + 0.375f), range);
			math::float3 v6  = reference_normal(math::float2(fx + 0.75f,  fy + 0.375f), range);
			math::float3 v7  = reference_normal(math::float2(fx + 0.875f, fy + 0.375f), range);

			math::float3 v8  = reference_normal(math::float2(fx + 0.125f, fy + 0.75f), range);
			math::float3 v9  = reference_normal(math::float2(fx + 0.375f, fy + 0.75f), range);
			math::float3 v10 = reference_normal(math::float2(fx + 0.75f,  fy + 0.75f), range);
			math::float3 v11 = reference_normal(math::float2(fx + 0.875f, fy + 0.75f), range);

			math::float3 v12 = reference_normal(math::float2(fx + 0.125f, fy + 0.875f), range);
			math::float3 v13 = reference_normal(math::float2(fx + 0.375f, fy + 0.875f), range);
			math::float3 v14 = reference_normal(math::float2(fx + 0.75f,  fy + 0.875f), range);
			math::float3 v15 = reference_normal(math::float2(fx + 0.875f, fy + 0.875f), range);

			math::float3 v = math::normalized(0.0625f * (v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 +
														 v8 + v9 + v10 + v11 + v12 + v13 + v14 + v15));

		//	math::float3 v = reference_normal(math::float2(fx + 0.5f, fy + 0.5f), range);

			pixel.x = color::float_to_snorm(v.x);
			pixel.y = color::float_to_snorm(v.y);
			pixel.z = color::float_to_snorm(v.z);
			pixel.w = 255;
		}
	}

	std::ofstream stream("reference_normal.png", std::ios::binary);
	if (!stream) {
		return;
	}

	size_t buffer_len = 0;
	void* png_buffer = tdefl_write_image_to_png_file_in_memory(rgba, dimensions.x, dimensions.y, 4, &buffer_len);

	if (!png_buffer) {
		delete [] rgba;
		return;
	}

	stream.write(static_cast<char*>(png_buffer), buffer_len);

	mz_free(png_buffer);

	delete [] rgba;
}

}}
