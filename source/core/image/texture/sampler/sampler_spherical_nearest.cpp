#include "sampler_spherical_nearest.hpp"
#include "image/texture/texture_2d.hpp"
#include "base/math/math.hpp"
#include "base/math/vector.inl"

namespace image { namespace texture { namespace sampler {

math::float3 Spherical_nearest::sample_3(const Texture_2D& texture, const math::float3& xyz) const {
	math::float2 uv((std::atan2(xyz.x, xyz.z) * math::Pi_inv + 1.f) * 0.5f, std::acos(xyz.y) * math::Pi_inv);

	const math::int2 d = texture.dimensions();

	int32_t x = std::min(static_cast<int32_t>(uv.x * static_cast<float>(d.x)), d.x - 1);
	int32_t y = std::min(static_cast<int32_t>(uv.y * static_cast<float>(d.y)), d.y - 1);

	return texture.at_3(x, y);
}

}}}
