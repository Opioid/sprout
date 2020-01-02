#include "ao.hpp"
#include "base/math/sampling.inl"
#include "base/math/vector4.inl"
#include "base/memory/align.hpp"
#include "base/random/generator.inl"
#include "rendering/integrator/surface/surface_integrator.inl"
#include "rendering/rendering_worker.hpp"
#include "scene/material/material.hpp"
#include "scene/material/material_sample.inl"
#include "scene/prop/prop_intersection.inl"
#include "scene/scene_ray.inl"

namespace rendering::integrator::surface {

AO::AO(rnd::Generator& rng, Settings const& settings) noexcept
    : Integrator(rng), settings_(settings), sampler_(rng) {}

void AO::prepare(Scene const& /*scene*/, uint32_t num_samples_per_pixel) noexcept {
    sampler_.resize(num_samples_per_pixel, settings_.num_samples, 1, 1);
}

void AO::start_pixel() noexcept {
    sampler_.start_pixel();
}

float4 AO::li(Ray& ray, Intersection& intersection, Worker& worker,
              Interface_stack const& initial_stack) noexcept {
    worker.reset_interface_stack(initial_stack);

    float const num_samples_reciprocal = 1.f / float(settings_.num_samples);

    float result = 0.f;

    float3 const wo = -ray.direction;

    auto const& material_sample = intersection.sample(wo, ray, Filter::Undefined, false, sampler_,
                                                      worker);

    Ray occlusion_ray;
    occlusion_ray.origin = material_sample.offset_p(intersection.geo.p);
    occlusion_ray.min_t  = 0.f;
    occlusion_ray.max_t  = settings_.radius;
    occlusion_ray.time   = ray.time;

    for (uint32_t i = settings_.num_samples; i > 0; --i) {
        float2 const sample = sampler_.generate_sample_2D();

        //		float3 ws = intersection.geo.tangent_to_world(hs);

        float3 const& n = material_sample.base_shading_normal();

        auto const tb = orthonormal_basis(n);

        float3 const ws = sample_oriented_hemisphere_cosine(sample, tb.a, tb.b, n);

        occlusion_ray.set_direction(ws);

        if (float mv; worker.masked_visibility(occlusion_ray, Filter::Undefined, mv)) {
            result += num_samples_reciprocal;
        }
    }

    return float4(result, result, result, 1.f);
}

AO_pool::AO_pool(uint32_t num_integrators, uint32_t num_samples, float radius) noexcept
    : Typed_pool<AO>(num_integrators) {
    settings_.num_samples = num_samples;
    settings_.radius      = radius;
}

Integrator* AO_pool::create(uint32_t id, rnd::Generator& rng) const noexcept {
    return new (&integrators_[id]) AO(rng, settings_);
}

}  // namespace rendering::integrator::surface
