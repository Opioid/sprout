#include "image4.hpp"

namespace image {

Image4::Image4(const Description& description) :
	Image(description),
	data_(new math::float4[description.dimensions.x * description.dimensions.y]) {}

Image4::~Image4() {
	delete [] data_;
}

math::float3 Image4::at3(uint32_t index) const {
	return data_[index].xyz;
}

math::float4 Image4::at4(uint32_t index) const {
	return data_[index];
}

void Image4::set3(uint32_t index, const math::float3& value) {
	data_[index].xyz = value;
}

void Image4::set4(uint32_t index, const math::float4& value) {
	data_[index] = value;
}

}
