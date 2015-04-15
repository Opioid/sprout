#include "image3.hpp"
#include "base/math/vector.inl"

namespace image {

Image3::Image3(const Description& description) :
	Image(description),
	data_(new math::float3[description.dimensions.x * description.dimensions.y]) {}

Image3::~Image3() {
	delete [] data_;
}

math::float3 Image3::at3(uint32_t index) const {
	return data_[index];
}

math::float4 Image3::at4(uint32_t index) const {
	return math::float4(data_[index], 1.f);
}

void Image3::set3(uint32_t index, const math::float3& value) {
	data_[index] = value;
}

void Image3::set4(uint32_t index, const math::float4& value) {
	data_[index] = value.xyz;
}

}
