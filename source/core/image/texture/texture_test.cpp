#include "texture_test.hpp"
#include "image/typed_image.inl"
#include "base/math/vector.inl"
#include <iostream>

namespace image { namespace texture { namespace testing {

bool is_valid_normal_map(const Image& image, const std::string& filename) {
	if (Image::Type::Byte_3 != image.description().type) {
		std::cout << "\"" << filename << "\" is not Byte_3" << std::endl;
		return false;
	}

	const Image_byte_3& typed_image = dynamic_cast<const Image_byte_3&>(image);

	int2 dimensions = image.description().dimensions;
	for (int32_t y = 0; y < dimensions.y; ++y) {
		for (int32_t x = 0; x < dimensions.x; ++x) {
			auto pixel = typed_image.at(x, y);

			if (0 == pixel.x + pixel.y + pixel.z) {
				std::cout << "\"" << filename << "\" [" << x << ", "
						  << y << "] is zero." << std::endl;
				return false;
			}
		}
	}

	return true;
}

}}}

