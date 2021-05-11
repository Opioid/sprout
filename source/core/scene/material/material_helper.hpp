#ifndef SU_CORE_SCENE_MATERIAL_HELPER_HPP
#define SU_CORE_SCENE_MATERIAL_HELPER_HPP

#include "base/math/vector3.inl"
#include "image/texture/texture.hpp"
#include "image/texture/texture_sampler.hpp"
#include "material_sample_helper.hpp"
#include "scene/scene_renderstate.hpp"

#ifdef SU_DEBUG
#include "scene/material/material_test.hpp"
#endif
#include "base/debug/assert.hpp"

namespace scene::material {

using Texture            = image::texture::Texture;
using Texture_sampler_2D = image::texture::Sampler_2D;

static inline float3 sample_normal(float3_p wo, Renderstate const& rs, float2 const uv,
                                   Texture const& map, Texture_sampler_2D const& sampler,
                                   Scene const& scene) {
    float2 const nm  = sampler.sample_2(map, uv, scene);
    float const  nmz = math::sqrt(std::max(1.f - dot(nm, nm), Dot_min));
    float3 const n   = normalize(rs.tangent_to_world(float3(nm, nmz)));

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

static inline float3 sample_normal(float3_p wo, Renderstate const& rs, Texture const& map,
                                   Texture_sampler_2D const& sampler, Scene const& scene) {
    return sample_normal(wo, rs, rs.uv, map, sampler, scene);
}

static inline float non_symmetry_compensation(float3_p wi, float3_p wo, float3_p geo_n,
                                              float3_p n) {
    // Veach's compensation for "Non-symmetry due to shading normals".
    // See e.g. CorrectShadingNormal() at:
    // https://github.com/mmp/pbrt-v3/blob/master/src/integrators/bdpt.cpp#L55

    float const numer = std::abs(dot(wo, geo_n) * dot(wi, n));
    float const denom = std::max(std::abs(dot(wo, n) * dot(wi, geo_n)), Dot_min);

    return std::min(numer / denom, 8.f);

    //    float const numer = scene::material::clamp_abs_dot(wi, n);
    //    float const denom = scene::material::clamp_abs_dot(wi, geo_n);

    //    return std::min(numer / denom, 1.f);
}

}  // namespace scene::material

#endif
