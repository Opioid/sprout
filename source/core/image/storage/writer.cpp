#include "writer.hpp"
#include "image/buffer/buffer.hpp"
#include "base/color/color.hpp"
#include "miniz/miniz.c"
#include <fstream>

namespace image {

bool write(const std::string& filename, const Buffer& buffer) {
	std::ofstream stream(filename, std::ios::binary);

	if (!stream) {
		return false;
	}

	auto& dimensions = buffer.dimensions();

	uint8_t* rgba = new uint8_t[dimensions.x * dimensions.y * 4];

	for (uint32_t y = 0; y < dimensions.y; ++y) {
		for (uint32_t x = 0; x < dimensions.x; ++x) {
			math::float4 color = buffer.at4(x, y);
			math::float3 sRGB = color::linear_to_sRGB(color.xyz);
			uint32_t i = (dimensions.x * y + x) * 4;
			rgba[i + 0] = uint8_t(sRGB.x * 255.f);
			rgba[i + 1] = uint8_t(sRGB.y * 255.f);
			rgba[i + 2] = uint8_t(sRGB.z * 255.f);
			rgba[i + 3] = uint8_t(color.w * 255.f);
		}
	}

	size_t buffer_len = 0;
	void* png_buffer = tdefl_write_image_to_png_file_in_memory(rgba, dimensions.x, dimensions.y, 4, &buffer_len);

	delete [] rgba;

	if (!png_buffer) {
		return false;
	}

	stream.write(static_cast<char*>(png_buffer), buffer_len);

	mz_free(png_buffer);

	return true;
}

}
