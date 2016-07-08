#include "flakes_provider.hpp"
#include "image/typed_image.inl"
#include "image/encoding/png/png_writer.hpp"
#include "image/procedural/image_renderer.hpp"

#include <iostream>

namespace image { namespace procedural { namespace flakes {

std::shared_ptr<Image> Provider::create(const memory::Variant_map& options) {

	std::cout << "Let's create some flakes" << std::endl;

	int2 dimensions(32, 32);

	Renderer renderer(dimensions);

	std::shared_ptr<Image_byte_3> image = std::make_shared<Image_byte_3>(
				Image::Description(Image::Type::Byte_3, dimensions));


	float3 normal(0.f, 0.f, 1.f);

	renderer.set_brush(normal);
	renderer.clear();

	normal = /*math::normalized*/(float3(1.f, 1.f, 1.f));
	renderer.set_brush(normal);
	renderer.draw_circle(float2(0.5f, 0.5f), 0.2f);

	/*
	math::byte3 color(127, 127, 255);

	for (int32_t i = 0, len = dimensions.x * dimensions.y; i < len; ++i) {

		image->at(i) = color;
	}*/

	renderer.resolve(*image);

	encoding::png::Writer::write("flakes.png", *image);

	return image;
}

}}}
