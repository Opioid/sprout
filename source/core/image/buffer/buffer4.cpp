#include "buffer4.hpp"
#include "base/math/vector.inl"

namespace image {

Buffer4::Buffer4(const math::uint2& dimensions) : Buffer(dimensions), data_(new math::float4[dimensions.x * dimensions.y]) {}

Buffer4::~Buffer4() {
	delete [] data_;
}

math::float4 Buffer4::at4(uint32_t x, uint32_t y) const {
	uint32_t i = dimensions_.x * y + x;
	return data_[i];
}

void Buffer4::set4(uint32_t x, uint32_t y, const math::float4& color) {
	uint32_t i = dimensions_.x * y + x;
	data_[i] = color;
}

}
