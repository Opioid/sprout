#include "substitute_material.hpp"
#include "base/math/vector4.inl"
#include "base/random/generator.inl"
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
                                         Worker const& worker) const {
    SOFT_ASSERT(!rs.subsurface);

    auto& sample = worker.sample<Sample>();

    auto const& sampler = worker.sampler_2D(sampler_key(), filter);

    set_sample(wo, rs, rs.ior, sampler, worker, sample);

    return sample;
}

size_t Material::num_bytes() const {
    return sizeof(*this);
}

size_t Material::sample_size() {
    return sizeof(Sample);
}

Frozen::Frozen(Sampler_settings const& sampler_settings, bool two_sided)
    : Material_base(sampler_settings, two_sided) {}

material::Sample const& Frozen::sample(float3 const& wo, Ray const& /*ray*/, Renderstate const& rs,
                                       Filter        filter, Sampler& /*sampler*/,
                                       Worker const& worker) const {
    SOFT_ASSERT(!rs.subsurface);

    auto& sample = worker.sample<Sample>();

    auto const& sampler = worker.sampler_2D(sampler_key(), filter);

    set_sample(wo, rs, rs.ior, sampler, worker, sample);

    float2 const r2(worker.rng().random_float(), worker.rng().random_float());

    float3 const dir = sample_hemisphere_cosine(r2);

    Ray const snow_ray(rs.p, normalize(float3(dir[0], 10.f * dir[2], dir[1])), 0.1f, 4.f);

    if (worker.visibility(snow_ray)) {
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

        sample.base_.diffuse_color_ = lerp(sample.base_.diffuse_color_, float3(1.f), weight);

        sample.base_.f0_ = lerp(sample.base_.f0_, float3(fresnel::schlick_f0(1.31f, rs.ior)),
                                weight);

        sample.base_.alpha_ = lerp(sample.base_.alpha_, alpha, weight);

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

size_t Frozen::num_bytes() const {
    return sizeof(*this);
}

size_t Frozen::sample_size() {
    return sizeof(Sample);
}

}  // namespace scene::material::substitute
