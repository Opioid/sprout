#include "image_1.hpp"
#include "base/math/vector.inl"

namespace image {

Image_1::Image_1(const Description& description) :
	Image(description),
	data_(new float[description.dimensions.x * description.dimensions.y]) {}

Image_1::~Image_1() {
	delete [] data_;
}

float Image_1::at1(uint32_t index) const {
	return data_[index];
}

math::float2 Image_1::at2(uint32_t index) const {
	return math::float2(data_[index], 0.f);
}

math::float3 Image_1::at3(uint32_t index) const {
	return math::float3(data_[index], 0.f, 0.f);
}

math::float4 Image_1::at4(uint32_t index) const {
	return math::float4(data_[index], 0.f, 0.f, 1.f);
}

void Image_1::set1(uint32_t index, float value) {
	data_[index] = value;
}

void Image_1::set3(uint32_t index, const math::float3& value) {
	data_[index] = value.x;
}

void Image_1::set4(uint32_t index, const math::float4& value) {
	data_[index] = value.x;
}

}

