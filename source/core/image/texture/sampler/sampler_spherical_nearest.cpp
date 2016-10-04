#include "sampler_spherical_nearest.hpp"
#include "image/texture/texture.hpp"
#include "base/math/math.hpp"
#include "base/math/vector.inl"

namespace image { namespace texture { namespace sampler {

float3 Spherical_nearest::sample_3(const Texture& texture, const float3& xyz) const {
	float2 uv((std::atan2(xyz.x, xyz.z) * math::Pi_inv + 1.f) * 0.5f, std::acos(xyz.y) * math::Pi_inv);

	const int2 d = texture.dimensions();

	int32_t x = std::min(static_cast<int32_t>(uv.x * static_cast<float>(d.x)), d.x - 1);
	int32_t y = std::min(static_cast<int32_t>(uv.y * static_cast<float>(d.y)), d.y - 1);

	return texture.at_3(x, y);
}

}}}
