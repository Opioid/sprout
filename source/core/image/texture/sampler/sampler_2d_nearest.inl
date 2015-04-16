#pragma once

#include "sampler_2d_nearest.hpp"
#include "image/texture/texture_2d.hpp"
#include <algorithm>

namespace image { namespace sampler {

template<typename Address_mode>
math::float3 Sampler_2D_nearest<Address_mode>::sample3(const Texture_2D& texture, math::float2 uv) const {
	const math::uint2 d = texture.dimensions();

	uv = address_mode_.f(uv);

	uint32_t x = std::min(static_cast<uint32_t>(uv.x * static_cast<float>(d.x)), d.x - 1);
	uint32_t y = std::min(static_cast<uint32_t>(uv.y * static_cast<float>(d.y)), d.y - 1);

	return texture.at3(x, y);
}

}}
