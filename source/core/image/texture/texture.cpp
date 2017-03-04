#include "texture.hpp"
#include "image/image.hpp"
#include "base/math/vector.inl"

namespace image { namespace texture {

Texture::Texture(std::shared_ptr<Image> image) :
	untyped_image_(image),
	back_(int3(image->description().dimensions.x - 1,
			   image->description().dimensions.y - 1,
			   image->description().dimensions.z - 1)),
	dimensions_float_(float3(image->description().dimensions)) {}

Texture::~Texture() {}

const Image* Texture::image() const {
	return untyped_image_.get();
}

int2 Texture::dimensions_2() const {
	return untyped_image_->description().dimensions.xy;
}

int3 Texture::dimensions_3() const {
	return untyped_image_->description().dimensions;
}

int2 Texture::back_2() const {
	return back_.xy;
}

int3 Texture::back_3() const {
	return back_;
}

float2 Texture::dimensions_float2() const {
	return dimensions_float_.xy();
}

float3 Texture::dimensions_float3() const {
	return dimensions_float_;
}

int32_t Texture::num_elements() const {
	return untyped_image_->description().num_elements;
}

float3 Texture::average_3() const {
	float3 average(0.f);

	const auto d = dimensions_2();
	for (int32_t y = 0; y < d.v[1]; ++y) {
		for (int32_t x = 0; x < d.v[0]; ++x) {
			average += at_3(x, y);
		}
	}

	const auto df = dimensions_float2();
	return average / (df.v[0] * df.v[1]);
}

float3 Texture::average_3(int32_t element) const {
	float3 average(0.f);

	const auto d = dimensions_2();
	for (int32_t y = 0; y < d.v[1]; ++y) {
		for (int32_t x = 0; x < d.v[0]; ++x) {
			average += at_element_3(x, y, element);
		}
	}

	const auto df = dimensions_float2();
	return average / (df.v[0] * df.v[1]);
}

float4 Texture::average_4() const {
	float4 average(0.f);

	const auto d = dimensions_2();
	for (int32_t y = 0; y < d.v[1]; ++y) {
		for (int32_t x = 0; x < d.v[0]; ++x) {
			average += at_4(x, y);
		}
	}

	const auto df = dimensions_float2();
	return average / (df.v[0] * df.v[1]);
}

}}
