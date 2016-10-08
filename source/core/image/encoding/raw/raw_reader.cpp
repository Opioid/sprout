#include "raw_reader.hpp"
#include "image/typed_image.inl"
#include "base/math/vector.inl"
#include <istream>
#include <string>

namespace image { namespace encoding { namespace raw {


std::shared_ptr<Image> Reader::read(std::istream& stream) const {
	int3 dimensions(32, 32, 32);

	Image::Description description(Image::Type::Byte_1, dimensions);

	auto volume = std::make_shared<Image_byte_1>(description);

	size_t num_bytes = dimensions.x * dimensions.y * dimensions.z;
	stream.read(reinterpret_cast<char*>(volume->data()), num_bytes);

	return volume;

	/*
	int3 dimensions(3, 3, 3);

	Image::Description description(Image::Type::Float_1, dimensions);

	auto volume = std::make_shared<Image_float_1>(description);

	volume->at(0, 0, 0) = 0.1f; volume->at(1, 0, 0) = 0.2f; volume->at(2, 0, 0) = 0.3f;
	volume->at(0, 1, 0) = 2.25f;  volume->at(1, 1, 0) = 2.f;  volume->at(2, 0, 0) = 1.75f;
	volume->at(0, 2, 0) = 0.25f;  volume->at(1, 2, 0) = 0.5f;  volume->at(2, 2, 0) = 0.75f;

	volume->at(0, 0, 1) = 0.4f; volume->at(1, 0, 1) = 0.5f; volume->at(2, 0, 1) = 0.6f;
	volume->at(0, 1, 1) = 1.5f;  volume->at(1, 1, 1) = 1.25f;  volume->at(2, 1, 1) = 1.f;
	volume->at(0, 2, 1) = 1.f;  volume->at(1, 2, 1) = 1.25f;  volume->at(2, 2, 1) = 1.5f;

	volume->at(0, 0, 2) = 0.7f; volume->at(1, 0, 2) = 0.8f; volume->at(2, 0, 2) = 0.9f;
	volume->at(0, 1, 2) = 0.75f;  volume->at(1, 1, 2) = 0.5f;  volume->at(2, 1, 2) = 0.25f;
	volume->at(0, 2, 2) = 1.75f;  volume->at(1, 2, 2) = 2.f;  volume->at(2, 2, 2) = 2.25f;

	return volume;
	*/
}

}}}
