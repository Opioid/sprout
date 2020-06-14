#ifndef SU_CORE_SCENE_MATERIAL_SUBSTITUTE_BASE_MATERIAL_INL
#define SU_CORE_SCENE_MATERIAL_SUBSTITUTE_BASE_MATERIAL_INL

#include "base/math/vector3.inl"
#include "image/texture/texture_adapter.inl"
#include "scene/material/fresnel/fresnel.inl"
#include "scene/material/ggx/ggx.inl"
#include "scene/material/material_helper.hpp"
#include "scene/material/material_sample.inl"
#include "scene/scene_renderstate.hpp"
#include "substitute_base_material.hpp"
#include "substitute_sample.hpp"
#include "scene/scene_worker.inl"

namespace scene::material::substitute {

static inline float3 f0_to_a(float3 const& f0) {
    return 5.f * sqrt(max(0.95f - f0, 0.0001f));
}

static inline float3 f0_to_a_b(float3 const& f0) {
    return 6.f * (1.f - f0);
}

// https://www.iquilezles.org/www/articles/checkerfiltering/checkerfiltering.htm

static inline float checkers(float2 uv) {
    float const a = sign(frac(uv[0] * 0.5f) - 0.5f);
    float const b = sign(frac(uv[1] * 0.5f) - 0.5f);

    return 0.5f - 0.5f * a * b;
}

// triangular signal
static inline float2 tri(float2 x ) {
    float hx = frac(x[0] * 0.5f) - 0.5f;
    float hy = frac(x[1] * 0.5f) - 0.5f;
    return float2(1.f - 2.f * std::abs(hx), 1.f - 2.f* std::abs(hy));
}

static inline float checkersGrad(float2 uv, float w) {
  //  vec2 w = max(abs(ddx), abs(ddy)) + 0.01;    // filter kernel
    float2 const i = (tri(uv + 0.5f * w) - tri(uv -0.5f * w)) / w;   // analytical integral (box filter)
    return 0.5f - 0.5f * i[0] * i[1];                   // xor pattern
}

static inline float3 intersect_plane(float3 const& plane_p, float3 const& plane_n, float3 const& origin, float3 const& direction) {
    float const d     = dot(plane_n, plane_p);
    float const denom = -dot(plane_n, direction);
    float const numer = dot(plane_n, origin) - d;
    float const hit_t = numer / denom;

    return origin + hit_t * direction;
}


template <typename Sample>
void Material_base::set_sample(float3 const& wo, Ray const& ray, Renderstate const& rs, float ior_outside,
                               Texture_sampler_2D const& sampler, Worker const& worker,
                               Sample& sample) const {
    sample.set_basis(rs.geo_n, rs.n, wo);

    if (normal_map_.is_valid()) {
        float3 const n = sample_normal(wo, rs, normal_map_, sampler, worker);
        sample.layer_.set_tangent_frame(n);
    } else {
        sample.layer_.set_tangent_frame(rs.t, rs.b, rs.n);
    }

    float3 color;
    if (color_map_.is_valid()) {
     //   color = color_map_.sample_3(worker, sampler, rs.uv);


        // https://blog.yiningkarlli.com/2018/10/bidirectional-mipmap.html

        // anti-aliased checker hack
        Ray_differential const rd = worker.camera().calculate_ray_differential(rs.p, ray.time, worker.scene());


        float3 const x_p = intersect_plane(rs.p, rs.geo_n, rd.x_origin, rd.x_direction);
        float3 const y_p = intersect_plane(rs.p, rs.geo_n, rd.y_origin, rd.y_direction);


        float const x_sd = squared_distance(rs.p, x_p);
        float const y_sd = squared_distance(rs.p, y_p);

        float const d = std::sqrt(std::max(x_sd, y_sd));


        float const w = d;//0.0001f;

        float const t = checkersGrad(2.f * rs.uv, w);

        color = lerp(float3(1.f), float3(0.f), t);

    } else {
        color = color_;
    }

    float2 surface;
    if (surface_map_.is_valid()) {
        surface = surface_map_.sample_2(worker, sampler, rs.uv);

        float const r = ggx::map_roughness(surface[0]);

        surface[0] = r * r;
    } else {
        surface[0] = alpha_;
        surface[1] = metallic_;
    }

    float3 radiance;
    if (emission_map_.is_valid()) {
        radiance = emission_factor_ * emission_map_.sample_3(worker, sampler, rs.uv);
    } else {
        radiance = float3(0.f);
    }

    sample.set_radiance(radiance);
    sample.base_.set(color, fresnel::schlick_f0(ior_, ior_outside), surface[0], surface[1],
                     rs.avoid_caustics);
}

}  // namespace scene::material::substitute

#endif
