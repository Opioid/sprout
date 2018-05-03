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

static inline float3 sample_normal(f_float3 wo,
								   const Renderstate& rs,
								   const image::texture::Adapter& map,
								   const image::texture::sampler::Sampler_2D& sampler) {
	float3 const nm = map.sample_3(sampler, rs.uv);
	float3 const n  = math::normalize(rs.tangent_to_world(nm));

	SOFT_ASSERT(testing::check_normal_map(n, nm, rs.uv));

	// Normal mapping can lead to normals facing away from the view direction.
	// I believe the following is the (imperfect) workaround referred to as "flipping" by
	// "Microfacet-based Normal Mapping for Robust Monte Carlo Path Tracing"
	// https://drive.google.com/file/d/0BzvWIdpUpRx_ZHI1X2Z4czhqclk/view
	if (math::dot(n, wo) < 0.f) {
		return math::reflect(rs.geo_n, n);
	}

	return n;
}

}

#endif
