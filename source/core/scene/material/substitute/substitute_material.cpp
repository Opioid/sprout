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

Frozen::Frozen(Sampler_settings const& sampler_settings, bool two_sided)
    : Material_base(sampler_settings, two_sided) {}

material::Sample const& Frozen::sample(float3 const& wo, Ray const& /*ray*/, Renderstate const& rs,
                                       Filter filter, Sampler& /*sampler*/, Worker& worker) const {
    SOFT_ASSERT(!rs.subsurface);

    auto& sample = worker.sample<Sample>();

    auto const& sampler = worker.sampler_2D(sampler_key(), filter);

    set_sample(wo, rs, rs.ior, sampler, worker, sample);

    float2 const r2(worker.rng().random_float(), worker.rng().random_float());

    float3 const dir = sample_hemisphere_cosine(r2);

    Ray const snow_ray(rs.p, normalize(float3(dir[0], 10.f * dir[2], dir[1])), 0.1f, 4.f);

    if (auto const v = worker.visibility(snow_ray, Filter::Undefined); v.valid) {
        float const angle = std::max(rs.n[1], 0.f);

        float const weight = angle * angle;

        float2 const uv(rs.p[0], rs.p[2]);

        if (snow_normal_map_.is_valid()) {
            auto const& repeating_sampler = worker.sampler_2D(3, Filter::Nearest);

            float3 const n = lerp(
                sample.layer_.n_,
                sample_normal(wo, rs, uv, snow_normal_map_, repeating_sampler, worker), weight);
            auto const [t, b] = orthonormal_basis(n);

            sample.layer_.set_tangent_frame(t, b, n);
        }

        float roughness = 0.9f;

        if (snow_mask_.is_valid()) {
            auto const& repeating_sampler = worker.sampler_2D(3, filter);

            float const snow_weight = snow_mask_.sample_1(worker, repeating_sampler, uv);

            roughness = lerp(roughness, 0.1f, snow_weight);
        }

        float const alpha = roughness * roughness;

        sample.base_.albedo_ = lerp(sample.base_.albedo_, float3(1.f), weight);

        sample.base_.f0_ = lerp(sample.base_.f0_, float3(fresnel::schlick_f0(1.31f, rs.ior)),
                                weight);

        sample.alpha_ = lerp(sample.alpha_, alpha, weight);

        sample.base_.metallic_ = lerp(sample.base_.metallic_, 0.f, weight);
    }

    return sample;
}

void Frozen::set_snow_normal_map(Texture_adapter const& normal_map) {
    snow_normal_map_ = normal_map;
}

void Frozen::set_snow_mask(Texture_adapter const& mask) {
    snow_mask_ = mask;
}

size_t Frozen::sample_size() {
    return sizeof(Sample);
}

}  // namespace scene::material::substitute
