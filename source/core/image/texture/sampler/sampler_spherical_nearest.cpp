#include "sampler_spherical_nearest.hpp"
#include "image/texture/texture2d.hpp"
#include "base/math/math.hpp"

namespace image { namespace sampler {

math::float3 Spherical_nearest::sample3(const Texture2D& texture, const math::float3& xyz) const {
	math::float2 uv((std::atan2(xyz.x, xyz.z) * math::pi_inv + 1.f) * 0.5f, std::acos(xyz.y) * math::pi_inv);

	const math::uint2 d = texture.dimensions();

	uint32_t x = std::min(static_cast<uint32_t>(uv.x * static_cast<float>(d.x)), d.x - 1);
	uint32_t y = std::min(static_cast<uint32_t>(uv.y * static_cast<float>(d.y)), d.y - 1);

	return texture.at3(x, y);
}

}}
