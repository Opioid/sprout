#include "texture_2d.hpp"
#include "image/image.hpp"
#include "base/math/vector.inl"

namespace image { namespace texture {

Texture_2D::Texture_2D(std::shared_ptr<Image> image) :
	untyped_image_(image),
	dimensions_float_(float2(image->description().dimensions)) {}

Texture_2D::~Texture_2D() {}

const Image* Texture_2D::image() const {
	return untyped_image_.get();
}

int2 Texture_2D::dimensions() const {
	return untyped_image_->description().dimensions;
}

float2 Texture_2D::dimensions_float() const {
	return dimensions_float_;
}

int32_t Texture_2D::num_elements() const {
	return untyped_image_->description().num_elements;
}

float3 Texture_2D::average_3() const {
	float3 average = math::float3_identity;

	auto d = dimensions();
	for (int32_t y = 0; y < d.y; ++y) {
		for (int32_t x = 0; x < d.x; ++x) {
			average += at_3(x, y);
		}
	}

	auto df = dimensions_float();
	return average / (df.x * df.y);
}

float3 Texture_2D::average_3(int32_t element) const {
	float3 average = math::float3_identity;

	auto d = dimensions();
	for (int32_t y = 0; y < d.y; ++y) {
		for (int32_t x = 0; x < d.x; ++x) {
			average += at_3(x, y, element);
		}
	}

	auto df = dimensions_float();
	return average / (df.x * df.y);
}

float4 Texture_2D::average_4() const {
	float4 average = math::float4_identity;

	auto d = dimensions();
	for (int32_t y = 0; y < d.y; ++y) {
		for (int32_t x = 0; x < d.x; ++x) {
			average += at_4(x, y);
		}
	}

	auto df = dimensions_float();
	return average / (df.x * df.y);
}

}}
