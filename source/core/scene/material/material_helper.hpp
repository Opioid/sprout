#ifndef SU_CORE_SCENE_MATERIAL_HELPER_HPP
#define SU_CORE_SCENE_MATERIAL_HELPER_HPP

#include "image/texture/texture_types.hpp"
#include "scene/scene_renderstate.hpp"
#include "base/math/vector3.inl"

#ifdef SU_DEBUG
#	include "scene/material/material_test.hpp"
#endif
#include "base/debug/assert.hpp"

namespace scene::material {

static inline float3 sample_normal(const image::texture::Adapter& map,
								   const image::texture::sampler::Sampler_2D& sampler,
								   const Renderstate& rs) {
	const float3 nm = map.sample_3(sampler, rs.uv);
	const float3 n  = math::normalize(rs.tangent_to_world(nm));

	SOFT_ASSERT(testing::check_normal_map(n, nm, rs.uv));

	return n;
}

}

#endif
