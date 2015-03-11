#include "buffer4.hpp"
#include "base/math/vector.inl"

namespace image {

math::float4 Buffer4::at4(uint32_t x, uint32_t y) const {
	return math::float4(1.f, 0.f, 0.f, 1.f);
}

void Buffer4::set4(const math::float4& color, uint32_t x, uint32_t y) {}

}
