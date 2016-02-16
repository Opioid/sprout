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

//	return math::normalized(v);

	v = math::normalized(v);

	return 0.5f * math::float3(v.x + 1.f, v.y + 1.f, v.z + 1.f);
}

void create_reference_normal_map(math::int2 dimensions) {
	color::Color4c* rgba = new color::Color4c[dimensions.x * dimensions.y];

	math::float2 range(2.f / static_cast<float>(dimensions.x),
					   2.f / static_cast<float>(dimensions.y));

	math::int2 aa(16, 16);

	math::float2 aa_delta = 1.f / math::float2(aa);
	math::float2 aa_offset = 0.5f * aa_delta;

	for (int32_t y = 0; y < dimensions.y; ++y) {
		for (int32_t x = 0; x < dimensions.x; ++x) {
			auto& pixel = rgba[y * dimensions.x + x];

			float fx = static_cast<float>(x);
			float fy = static_cast<float>(y);

			math::float3 v = math::float3_identity;

			for (int32_t ay = 0; ay < aa.y; ++ay) {
				for (int32_t ax = 0; ax < aa.x; ++ax) {
					v += reference_normal(math::float2(fx + aa_offset.x + static_cast<float>(ax) * aa_delta.x,
													   fy + aa_offset.y + static_cast<float>(ay) * aa_delta.y),
										  range);
				}
			}

//			v = math::normalized(v / static_cast<float>(aa.x * aa.y));

//			pixel.x = color::float_to_snorm(v.x);
//			pixel.y = color::float_to_snorm(v.y);
//			pixel.z = color::float_to_snorm(v.z);

			v = v / static_cast<float>(aa.x * aa.y);

			pixel.x = static_cast<uint8_t>(v.x * 255.f);
			pixel.y = static_cast<uint8_t>(v.y * 255.f);
			pixel.z = static_cast<uint8_t>(v.z * 255.f);

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
