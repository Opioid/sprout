#pragma once

#include "texture.hpp"
#include "base/math/vector.hpp"

namespace image { namespace texture {

class Texture_2D : public Texture {
public:

	Texture_2D(std::shared_ptr<Image> image);

	math::uint2 dimensions() const;

	float        at1(uint32_t x, uint32_t y) const;
	math::float2 at2(uint32_t x, uint32_t y) const;
	math::float3 at3(uint32_t x, uint32_t y) const;
	math::float4 at4(uint32_t x, uint32_t y) const;

	math::float4 average() const;
};

}}
