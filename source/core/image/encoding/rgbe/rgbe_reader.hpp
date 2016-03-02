#pragma once

#include "image/typed_image.hpp"
#include "base/math/vector.hpp"
#include <cstdint>
#include <istream>
#include <memory>

namespace image { namespace encoding { namespace rgbe {

class Reader {
public:

	std::shared_ptr<Image> read(std::istream& stream) const;

private:

	struct Header {
		uint32_t width;
		uint32_t height;
	};

	static Header read_header(std::istream& stream);

	static void read_pixels_RLE(std::istream& stream, uint32_t scanline_width, uint32_t num_scanlines, Image_float_3& image);
	static void read_pixels(std::istream& stream, uint32_t num_pixels, Image_float_3& image, uint32_t offset);

	static math::float3 rgbe_to_float3(uint8_t rgbe[4]);
};

}}}
