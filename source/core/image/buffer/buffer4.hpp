#pragma once

#include "buffer.hpp"

namespace image {

class Buffer4 : public Buffer {
public:

	Buffer4(const math::uint2& dimensions);

	virtual ~Buffer4();

	virtual math::float4 at4(uint32_t x, uint32_t y) const;
	virtual void set4(const math::float4& color, uint32_t x, uint32_t y);

private:

	math::float4* data_;
};

}
