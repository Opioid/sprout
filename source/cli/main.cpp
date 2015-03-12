#include "core/image/buffer/buffer4.hpp"
#include "core/image/storage/writer.hpp"
#include "base/math/vector.inl"
#include <iostream>

int main() {
	std::cout << "Welcome to sprout!" << std::endl;

	image::Buffer4 buffer(math::uint2(512, 512));

	auto& dimensions = buffer.dimensions();

	for (uint32_t y = 0; y < dimensions.y; ++y) {
		for (uint32_t x = 0; x < dimensions.x; ++x) {
			math::float4 color(float(x) / float(dimensions.x), float(y) / float(dimensions.y), 0.5f, 1.f);
			buffer.set4(color, x, y);
		}
	}

	bool result = image::write("output.png", &buffer);

	if (result) {
		std::cout << "We wrote output.png" << std::endl;
	} else {
		std::cout << "Something went wrong" << std::endl;
	}

	return 0;
}
