#include "ao.hpp"
#include "base/math/sampling.inl"
#include "base/math/vector4.inl"
#include "base/memory/align.hpp"
#include "base/random/generator.inl"
#include "rendering/rendering_worker.hpp"
#include "scene/material/material.hpp"
#include "scene/material/material_sample.inl"
#include "scene/prop/prop_intersection.inl"
#include "scene/scene_ray.inl"

namespace rendering::integrator::surface {

AO::AO(rnd::Generator& rng, take::Settings const& take_settings, Settings const& settings) noexcept
    : Integrator(rng, take_settings), settings_(settings), sampler_(rng) {}

void AO::prepare(Scene const& /*scene*/, uint32_t num_samples_per_pixel) noexcept {
    sampler_.resize(num_samples_per_pixel, settings_.num_samples, 1, 1);
}

void AO::resume_pixel(uint32_t sample, rnd::Generator& scramble) noexcept {
    sampler_.resume_pixel(sample, scramble);
}

float3 AO::li(Ray& ray, Intersection& intersection, Worker& worker,
              Interface_stack const& initial_stack) noexcept {
    worker.reset_interface_stack(initial_stack);

    float const num_samples_reciprocal = 1.f / static_cast<float>(settings_.num_samples);

    float result = 0.f;

    Ray occlusion_ray;
    occlusion_ray.origin = intersection.geo.p;
    occlusion_ray.min_t  = take_settings_.ray_offset_factor * intersection.geo.epsilon;
    occlusion_ray.max_t  = settings_.radius;
    occlusion_ray.time   = ray.time;

    float3 const wo = -ray.direction;

    auto const& material_sample = intersection.sample(wo, ray, Sampler_filter::Undefined, false,
                                                      sampler_, worker);

    for (uint32_t i = settings_.num_samples; i > 0; --i) {
        float2 const sample = sampler_.generate_sample_2D();
        float3 const hs     = math::sample_hemisphere_cosine(sample);
        //		float3 ws = intersection.geo.tangent_to_world(hs);
        float3 const ws = material_sample.base_layer().tangent_to_world(hs);

        occlusion_ray.set_direction(ws);

        if (float mv; worker.masked_visibility(occlusion_ray, Sampler_filter::Undefined, mv)) {
            result += num_samples_reciprocal;
        }
    }

    return float3(result);
}

size_t AO::num_bytes() const noexcept {
    return sizeof(*this) + sampler_.num_bytes();
}

AO_factory::AO_factory(take::Settings const& settings, uint32_t num_integrators,
                       uint32_t num_samples, float radius) noexcept
    : Factory(settings), integrators_(memory::allocate_aligned<AO>(num_integrators)) {
    settings_.num_samples = num_samples;
    settings_.radius      = radius;
}

AO_factory::~AO_factory() noexcept {
    memory::free_aligned(integrators_);
}

Integrator* AO_factory::create(uint32_t id, rnd::Generator& rng) const noexcept {
    return new (&integrators_[id]) AO(rng, take_settings_, settings_);
}

}  // namespace rendering::integrator::surface
