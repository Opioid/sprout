#pragma once

#include "base/math/vector.hpp"

namespace image {

class Buffer {
public:

	virtual math::float4 at4(uint32_t x, uint32_t y) const = 0;
	virtual void set4(const math::float4& color, uint32_t x, uint32_t y) = 0;
};

}
