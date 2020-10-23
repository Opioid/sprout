#include "ao.hpp"
#include "base/math/sampling.inl"
#include "base/math/vector4.inl"
#include "base/memory/align.hpp"
#include "base/random/generator.inl"
#include "rendering/integrator/surface/surface_integrator.inl"
#include "rendering/rendering_worker.hpp"
#include "sampler/sampler_golden_ratio.hpp"
#include "sampler/sampler_random.hpp"
#include "scene/material/material.hpp"
#include "scene/material/material_sample.inl"
#include "scene/prop/prop_intersection.inl"
#include "scene/scene_ray.inl"

namespace rendering::integrator::surface {

static sampler::Sampler* create_sampler(bool progressive) {
    if (progressive) {
        return new sampler::Random();
    }
    return new sampler::Golden_ratio();
}

AO::AO(Settings const& settings, bool progressive)
    : settings_(settings), sampler_(create_sampler(progressive)) {}

AO::~AO() {
    delete sampler_;
}

void AO::prepare(Scene const& /*scene*/, uint32_t num_samples_per_pixel) {
    sampler_->resize(num_samples_per_pixel, settings_.num_samples, 1, 1);
}

void AO::start_pixel(RNG& rng) {
    sampler_->start_pixel(rng);
}

float4 AO::li(Ray& ray, Intersection& isec, Worker& worker, Interface_stack const& initial_stack, AOV& aov) {
    worker.reset_interface_stack(initial_stack);

    float const num_samples_reciprocal = 1.f / float(settings_.num_samples);

    float result = 0.f;

    float3 const wo = -ray.direction;

    auto const& mat_sample = isec.sample(wo, ray, Filter::Undefined, false, *sampler_, worker);

    Ray occlusion_ray;
    occlusion_ray.origin  = mat_sample.offset_p(isec.geo.p, false, false);
    occlusion_ray.max_t() = settings_.radius;
    occlusion_ray.time    = ray.time;

    for (uint32_t i = settings_.num_samples; i > 0; --i) {
        float2 const sample = sampler_->generate_sample_2D(worker.rng());

        float3 const& t = mat_sample.shading_tangent();
        float3 const& b = mat_sample.shading_bitangent();
        float3 const& n = mat_sample.shading_normal();

        float3 const ws = sample_oriented_hemisphere_cosine(sample, t, b, n);

        occlusion_ray.set_direction(ws);

        if (auto const v = worker.visibility(occlusion_ray, Filter::Undefined); v.valid) {
            result += num_samples_reciprocal;
        }
    }

    return float4(result, result, result, 1.f);
}

AO_pool::AO_pool(uint32_t num_integrators, bool progressive, uint32_t num_samples, float radius)
    : Typed_pool<AO>(num_integrators), progressive_(progressive) {
    settings_.num_samples = num_samples;
    settings_.radius      = radius;
}

Integrator* AO_pool::get(uint32_t id) const {
    if (uint32_t const zero = 0;
        0 == std::memcmp(&zero, static_cast<void*>(&integrators_[id]), 4)) {
        return new (&integrators_[id]) AO(settings_, progressive_);
    }

    return &integrators_[id];
}

}  // namespace rendering::integrator::surface
