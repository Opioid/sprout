#include "sampler_2d_nearest.hpp"

namespace image { namespace sampler {

math::float3 Sampler_2D_nearest::sample3(const Texture_2D& texture, math::float2 uv) const {
	return math::float3(1.f, 0.f, 0.f);
}

}}
