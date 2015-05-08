#include "image_3.hpp"
#include "base/math/vector.inl"

namespace image {

Image_3::Image_3(const Description& description) :
	Image(description),
	data_(new math::float3[description.dimensions.x * description.dimensions.y]) {}

Image_3::~Image_3() {
	delete [] data_;
}

float Image_3::at1(uint32_t index) const {
	return data_[index].x;
}

math::float3 Image_3::at3(uint32_t index) const {
	return data_[index];
}

math::float4 Image_3::at4(uint32_t index) const {
	return math::float4(data_[index], 1.f);
}

void Image_3::set1(uint32_t index, float value) {
	data_[index].x = value;
}

void Image_3::set3(uint32_t index, const math::float3& value) {
	data_[index] = value;
}

void Image_3::set4(uint32_t index, const math::float4& value) {
	data_[index] = value.xyz;
}

}
