#include "buffer.hpp"
#include "base/math/vector.inl"

namespace image {

Buffer::Buffer(const math::uint2& dimensions) : dimensions_(dimensions) {}

Buffer::~Buffer() {}

const math::uint2& Buffer::dimensions() const {
	return dimensions_;
}

}
