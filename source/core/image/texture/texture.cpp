#include "texture.hpp"
#include "image/image.hpp"
#include "base/math/vector4.inl"

namespace image::texture {

Texture::Texture(const std::shared_ptr<Image>& image) :
	untyped_image_(image),
	back_(int3(image->description().dimensions[0] - 1,
			   image->description().dimensions[1] - 1,
			   image->description().dimensions[2] - 1)),
	dimensions_float_(float3(image->description().dimensions)) {}

Texture::~Texture() {}

const Image* Texture::image() const {
	return untyped_image_.get();
}

int32_t Texture::num_channels() const {
	return untyped_image_->description().num_channels();
}

int32_t Texture::width() const {
	return untyped_image_->description().dimensions[0];
}

int2 Texture::dimensions_2() const {
	return untyped_image_->description().dimensions.xy();
}

int3 Texture::dimensions_3() const {
	return untyped_image_->description().dimensions;
}

int2 Texture::back_2() const {
	return back_.xy();
}

int3 const& Texture::back_3() const {
	return back_;
}

float2 Texture::dimensions_float2() const {
	return dimensions_float_.xy();
}

float3 const& Texture::dimensions_float3() const {
	return dimensions_float_;
}

int32_t Texture::num_elements() const {
	return untyped_image_->description().num_elements;
}

float3 Texture::average_3() const {
	float3 average(0.f);

	auto const d = dimensions_2();
	for (int32_t y = 0; y < d[1]; ++y) {
		for (int32_t x = 0; x < d[0]; ++x) {
			average += at_3(x, y);
		}
	}

	auto const df = dimensions_float2();
	return average / (df[0] * df[1]);
}

float3 Texture::average_3(int32_t element) const {
	float3 average(0.f);

	auto const d = dimensions_2();
	for (int32_t y = 0; y < d[1]; ++y) {
		for (int32_t x = 0; x < d[0]; ++x) {
			average += at_element_3(x, y, element);
		}
	}

	auto const df = dimensions_float2();
	return average / (df[0] * df[1]);
}

float4 Texture::average_4() const {
	float4 average(0.f);

	auto const d = dimensions_2();
	for (int32_t y = 0; y < d[1]; ++y) {
		for (int32_t x = 0; x < d[0]; ++x) {
			average += at_4(x, y);
		}
	}

	auto const df = dimensions_float2();
	return average / (df[0] * df[1]);
}

}
