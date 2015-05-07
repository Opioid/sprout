#include "image_4.hpp"
#include "base/math/vector.inl"

namespace image {

Image_4::Image_4(const Description& description) :
	Image(description),
	data_(new math::float4[description.dimensions.x * description.dimensions.y]) {}

Image_4::~Image_4() {
	delete [] data_;
}

math::float3 Image_4::at3(uint32_t index) const {
	return data_[index].xyz;
}

math::float4 Image_4::at4(uint32_t index) const {
	return data_[index];
}

void Image_4::set3(uint32_t index, const math::float3& value) {
	data_[index].xyz = value;
}

void Image_4::set4(uint32_t index, const math::float4& value) {
	data_[index] = value;
}

}
