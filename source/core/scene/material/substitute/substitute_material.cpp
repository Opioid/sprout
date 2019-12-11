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

Material::Material(Sampler_settings const& sampler_settings, bool two_sided) noexcept
    : Material_base(sampler_settings, two_sided) {}

material::Sample const& Material::sample(float3 const&      wo, Ray const& /*ray*/,
                                         Renderstate const& rs, Filter                filter,
                                         sampler::Sampler& /*sampler*/, Worker const& worker) const
    noexcept {
    SOFT_ASSERT(!rs.subsurface);

    auto& sample = worker.sample<Sample>();

    auto& sampler = worker.sampler_2D(sampler_key(), filter);

    set_sample(wo, rs, rs.ior, sampler, worker, sample);

    return sample;
}

size_t Material::num_bytes() const noexcept {
    return sizeof(*this);
}

size_t Material::sample_size() noexcept {
    return sizeof(Sample);
}

Frozen::Frozen(Sampler_settings const& sampler_settings, bool two_sided) noexcept
    : Material_base(sampler_settings, two_sided) {}

material::Sample const& Frozen::sample(float3 const& wo, Ray const& /*ray*/, Renderstate const& rs,
                                       Filter        filter, sampler::Sampler& /*sampler*/,
                                       Worker const& worker) const noexcept {
    SOFT_ASSERT(!rs.subsurface);

    auto& sample = worker.sample<Sample>();

    auto& sampler = worker.sampler_2D(sampler_key(), filter);

    set_sample(wo, rs, rs.ior, sampler, worker, sample);

    float2 const r2(worker.rng().random_float(), worker.rng().random_float());

    float3 const dir = sample_hemisphere_cosine(r2);

    Ray const snow_ray(rs.p, normalize(float3(dir[0], 10.f * dir[2], dir[1])), 0.1f, 4.f);

    if (worker.visibility(snow_ray)) {
        if (snow_normal_map_.is_valid()) {
            auto& repeating_sampler = worker.sampler_2D(3, filter);

            float2 const uv(rs.p[0], rs.p[2]);
            float3 const n = sample_normal(wo, rs, uv, snow_normal_map_, repeating_sampler, worker);
            sample.layer_.set_tangent_frame(n);
        }

        float angle = std::max(sample.layer_.n_[1], 0.f);

        angle *= angle;

        sample.base_.diffuse_color_ = lerp(sample.base_.diffuse_color_, float3(1.f), angle);

        sample.base_.f0_ = lerp(sample.base_.f0_, float3(fresnel::schlick_f0(ior_, rs.ior)), angle);

        sample.base_.alpha_ = lerp(sample.base_.alpha_, 0.9f * 0.9f, angle);

        sample.base_.metallic_ = lerp(sample.base_.metallic_, 0.f, angle);
    }

    return sample;
}

void Frozen::set_snow_normal_map(Texture_adapter const& normal_map) noexcept {
    snow_normal_map_ = normal_map;
}

size_t Frozen::num_bytes() const noexcept {
    return sizeof(*this);
}

size_t Frozen::sample_size() noexcept {
    return sizeof(Sample);
}

}  // namespace scene::material::substitute
