#include "substitute_material.hpp"
#include "base/math/vector4.inl"
#include "base/random/generator.inl"
#include "scene/material/material.inl"
#include "scene/scene_ray.inl"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.inl"
#include "substitute_base_material.inl"
#include "substitute_base_sample.inl"
#include "substitute_sample.hpp"

#include "base/debug/assert.hpp"

namespace scene::material::substitute {

Material::Material(Sampler_settings const& sampler_settings, bool two_sided)
    : Material_base(sampler_settings, two_sided) {}

material::Sample const& Material::sample(float3 const&      wo, Ray const& /*ray*/,
                                         Renderstate const& rs, Filter filter, Sampler& /*sampler*/,
                                         Worker& worker) const {
    SOFT_ASSERT(!rs.subsurface);

    auto& sample = worker.sample<Sample>();

    auto const& sampler = worker.sampler_2D(sampler_key(), filter);

    set_sample(wo, rs, rs.ior, sampler, worker, sample);

    return sample;
}

size_t Material::sample_size() {
    return sizeof(Sample);
}

Checkers::Checkers(Sampler_settings const& sampler_settings, bool two_sided)
    : Material_base(sampler_settings, two_sided) {}

// https://www.iquilezles.org/www/articles/checkerfiltering/checkerfiltering.htm

// triangular signal
static inline float2 tri(float2 x) {
    float const hx = frac(x[0] * 0.5f) - 0.5f;
    float const hy = frac(x[1] * 0.5f) - 0.5f;
    return float2(1.f - 2.f * std::abs(hx), 1.f - 2.f * std::abs(hy));
}

static inline float checkers_grad(float2 uv, float2 ddx, float2 ddy) {
    // filter kernel
    float2 const w = max(abs(ddx), abs(ddy)) + 0.0001f;

    // analytical integral (box filter)
    float2 const i = (tri(uv + 0.5f * w) - tri(uv - 0.5f * w)) / w;

    // xor pattern
    return 0.5f - 0.5f * i[0] * i[1];
}

material::Sample const& Checkers::sample(float3 const& wo, Ray const& ray, Renderstate const& rs,
                                         Filter  filter, Sampler& /*sampler*/,
                                         Worker& worker) const {
    SOFT_ASSERT(!rs.subsurface);

    auto& sample = worker.sample<Sample>();

    auto const& sampler = worker.sampler_2D(sampler_key(), filter);

    if (normal_map_.is_valid()) {
        float3 const n = sample_normal(wo, rs, normal_map_, sampler, worker);
        sample.layer_.set_tangent_frame(n);
    } else {
        sample.layer_.set_tangent_frame(rs.t, rs.b, rs.n);
    }

    float4 const dd = scale_ * worker.screenspace_differential(rs, ray.time);

    float const t = checkers_grad(scale_ * rs.uv, dd.xy(), dd.zw());

    float3 const color = lerp(checkers_[0], checkers_[1], t);

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

    sample.set_common(rs, wo, color, radiance, surface[0]);
    sample.base_.set(color, fresnel::schlick_f0(ior_, rs.ior), surface[1]);

    return sample;
}

void Checkers::set_checkers(float3 const& a, float3 const& b, float scale) {
    checkers_[0] = a;
    checkers_[1] = b;

    scale_ = scale;
}

size_t Checkers::sample_size() {
    return sizeof(Sample);
}

}  // namespace scene::material::substitute
