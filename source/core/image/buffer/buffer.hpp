#pragma once

#include "base/math/vector.hpp"

namespace image {

class Buffer {
public:

	Buffer(const math::uint2& dimensions);

	virtual ~Buffer();

	const math::uint2& dimensions() const;

	virtual math::float4 at4(uint32_t x, uint32_t y) const = 0;
	virtual void set4(uint32_t x, uint32_t y, const math::float4& color) = 0;

protected:

	math::uint2 dimensions_;
};

}
