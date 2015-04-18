#include "image_writer.hpp"
#include "image/image.hpp"
#include "base/color/color.hpp"
#include "base/math/vector.inl"
#include "miniz/miniz.h"
#include <fstream>

namespace image {

bool write(const std::string& filename, const Image& image) {
	std::ofstream stream(filename, std::ios::binary);
	if (!stream) {
		return false;
	}

	math::uint2 dimensions = image.description().dimensions;

	color::Color4c* rgba = new color::Color4c[dimensions.x * dimensions.y];
//	uint32_t* rgba = new uint32_t[dimensions.x * dimensions.y];

	for (uint32_t y = 0, index = 0; y < dimensions.y; ++y) {
		for (uint32_t x = 0; x < dimensions.x; ++x, ++index) {
			math::float4 color = image.at4(index);
			color.xyz = color::linear_to_sRGB(color.xyz);
			rgba[index] = color::to_byte(color);
		//	rgba[index] = color::to_uint(color);
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
