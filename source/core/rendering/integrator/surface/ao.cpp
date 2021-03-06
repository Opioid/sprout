#include "ao.hpp"
#include "base/math/sampling.inl"
#include "base/math/vector4.inl"
#include "base/memory/align.hpp"
#include "base/random/generator.inl"
#include "rendering/integrator/integrator.inl"
#include "rendering/rendering_worker.hpp"
#include "rendering/sensor/aov/value.inl"
#include "sampler/sampler_golden_ratio.hpp"
#include "sampler/sampler_random.hpp"
#include "scene/material/material.inl"
#include "scene/material/material_sample.inl"
#include "scene/prop/prop_intersection.inl"
#include "scene/scene_ray.inl"

namespace rendering::integrator::surface {

static sampler::Sampler* create_sampler(uint32_t max_samples_per_pixel, bool progressive) {
    if (progressive) {
        return new sampler::Random();
    }
    return new sampler::Golden_ratio(1, 1, max_samples_per_pixel);
}

AO::AO(Settings const& settings, uint32_t max_samples_per_pixel, bool progressive)
    : settings_(settings),
      sampler_(create_sampler(settings.num_samples * max_samples_per_pixel, progressive)) {}

AO::~AO() {
    delete sampler_;
}

void AO::start_pixel(RNG& rng) {
    sampler_->start_pixel(rng);
}

float4 AO::li(Ray& ray, Intersection& isec, Worker& worker, Interface_stack const& initial_stack,
              AOV* aov) {
    worker.reset_interface_stack(initial_stack);

    float const num_samples_reciprocal = 1.f / float(settings_.num_samples);

    float result = 0.f;

    float3 const wo = -ray.direction;

    auto const& mat_sample = isec.sample(wo, ray, Filter::Undefined, 0.f, false, *sampler_, worker);

    if (aov) {
        common_AOVs(float3(1.f), ray, isec, mat_sample, true, worker, *aov);
    }

    Ray occlusion_ray;
    occlusion_ray.origin  = isec.offset_p(mat_sample.geometric_normal(), false);
    occlusion_ray.max_t() = settings_.radius;
    occlusion_ray.time    = ray.time;

    for (uint32_t i = settings_.num_samples; i > 0; --i) {
        float2 const sample = sampler_->sample_2D(worker.rng());

        float3 const t = mat_sample.shading_tangent();
        float3 const b = mat_sample.shading_bitangent();
        float3 const n = mat_sample.shading_normal();

        float3 const ws = sample_oriented_hemisphere_cosine(sample, t, b, n);

        occlusion_ray.set_direction(ws);

        //        if (auto const v = worker.visibility(occlusion_ray, Filter::Undefined); v.valid) {
        //            result += num_samples_reciprocal;
        //        }

        if (float3 v; worker.visibility(occlusion_ray, Filter::Undefined, v)) {
            result += num_samples_reciprocal;
        }
    }

    return float4(result, result, result, 1.f);
}

AO_pool::AO_pool(uint32_t num_integrators, bool progressive, uint32_t num_samples, float radius)
    : Typed_pool<AO, Integrator>(num_integrators), progressive_(progressive) {
    settings_.num_samples = num_samples;
    settings_.radius      = radius;
}

Integrator* AO_pool::create(uint32_t id, uint32_t max_samples_per_pixel) const {
    return new (&integrators_[id]) AO(settings_, max_samples_per_pixel, progressive_);
}

}  // namespace rendering::integrator::surface
