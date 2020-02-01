#ifndef SU_CORE_SCENE_MATERIAL_HELPER_HPP
#define SU_CORE_SCENE_MATERIAL_HELPER_HPP

#include "base/math/vector3.inl"
#include "image/texture/texture_adapter.hpp"
#include "image/texture/texture_types.hpp"
#include "scene/scene_renderstate.hpp"

#ifdef SU_DEBUG
#include "scene/material/material_test.hpp"
#endif
#include "base/debug/assert.hpp"

namespace scene::material {

using Texture_adapter    = image::texture::Adapter;
using Texture_sampler_2D = image::texture::Sampler_2D;

static inline float3 sample_normal(float3 const& wo, Renderstate const& rs, float2 const uv,
                                   Texture_adapter const& map, Texture_sampler_2D const& sampler,
                                   scene::Worker const& worker) noexcept {
    float3 const nm = map.sample_3(worker, sampler, uv);
    float3 const n  = normalize(rs.tangent_to_world(nm));

    SOFT_ASSERT(testing::check_normal_map(n, nm, uv));

    // Normal mapping can lead to normals facing away from the view direction.
    // I believe the following is the (imperfect) workaround referred to as "flipping" by
    // "Microfacet-based Normal Mapping for Robust Monte Carlo Path Tracing"
    // https://drive.google.com/file/d/0BzvWIdpUpRx_ZHI1X2Z4czhqclk/view
    if (dot(n, wo) < 0.f) {
        return reflect(rs.geo_n, n);
    }

    return n;
}

static inline float3 sample_normal(float3 const& wo, Renderstate const& rs,
                                   Texture_adapter const& map, Texture_sampler_2D const& sampler,
                                   scene::Worker const& worker) noexcept {
    return sample_normal(wo, rs, rs.uv, map, sampler, worker);
}

}  // namespace scene::material

#endif
