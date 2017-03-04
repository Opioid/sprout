#include "texture_test.hpp"
#include "texture_encoding.hpp"
#include "image/typed_image.inl"
#include "base/math/vector.inl"
#include "base/math/print.hpp"
#include <iostream>

namespace image { namespace texture { namespace testing {

bool is_valid_normal_map(const Image& image, const std::string& filename) {
	if (Image::Type::Byte_3 != image.description().type) {
		std::cout << "\"" << filename << "\" is not Byte_3" << std::endl;
		return false;
	}

	const Byte_3& typed_image = static_cast<const Byte_3&>(image);

	int2 d = image.description().dimensions.xy;
	for (int32_t y = 0; y < d.v[1]; ++y) {
		for (int32_t x = 0; x < d.v[0]; ++x) {
			auto value = typed_image.at(x, y);

			if (0 == value.x + value.y + value.z) {
				std::cout << "\"" << filename << "\" [" << x << ", "
						  << y << "] is zero." << std::endl;
				return false;
			}

			float3 n(encoding::cached_snorm_to_float(value.x),
					 encoding::cached_snorm_to_float(value.y),
					 encoding::cached_snorm_to_float(value.z));

			if (0.f == n.x && 0.f == n.y && 0.f == n.z) {
				std::cout << "\"" << filename << "\" [" << x << ", "
						  << y << "] is [0, 0, 0]." << std::endl;

				std::cout << "rgb: " << value << std::endl;

				return false;
			}

			/*
			float l = math::length(n);

			if (std::abs(l - 1.f) > 0.01f) {
				std::cout << "\"" << filename << "\" [" << x << ", "
						  << y << "] is degenerate normal." << std::endl;

				return false;
			}
			*/
		}
	}

	return true;
}

}}}

