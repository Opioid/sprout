#include "surface_integrator.hpp"
#include "base/math/sampling.inl"
#include "base/random/generator.inl"
#include "rendering/rendering_worker.inl"
#include "rendering/sensor/aov/value.inl"
#include "scene/material/material_sample.inl"
#include "scene/prop/prop_intersection.hpp"
#include "scene/scene_ray.inl"

namespace rendering::integrator::surface {

Integrator::Integrator() = default;

Integrator::~Integrator() = default;

void Integrator::common_AOVs(float3_p throughput, Ray const& ray, Intersection const& isec,
                             Material_sample const& mat_sample, bool primary_ray, Worker& worker,
                             AOV& aov) {
    using Property = sensor::aov::Property;

    if (primary_ray && mat_sample.can_evaluate()) {
        aov.insert(throughput * mat_sample.albedo(), Property::Albedo);
    }

    if (ray.depth > 0) {
        return;
    }

    if (aov.active(Property::Roughness)) {
        float const a = mat_sample.alpha();
        float const r = std::sqrt(a);
        aov.insert(float3(r), Property::Roughness);
    }

    if (aov.active(Property::Geometric_normal)) {
        aov.insert(mat_sample.geometric_normal(), Property::Geometric_normal);
    }

    if (aov.active(Property::Shading_normal)) {
        aov.insert(mat_sample.shading_normal(), Property::Shading_normal);
    }

    if (aov.active(Property::Material_id)) {
        aov.insert(float3(float(worker.scene().prop_material_id(isec.prop, isec.geo.part))),
                   Property::Material_id);
    }

    if (aov.active(Property::Depth)) {
        aov.insert(ray.max_t(), Property::Depth);
    }

    if (aov.active(Property::AO)) {
        Ray occlusion_ray;
        occlusion_ray.origin  = mat_sample.offset_p(isec.geo.p, false, false);
        occlusion_ray.max_t() = 1.f;  // settings_.radius;
        occlusion_ray.time    = ray.time;

        float const num_samples_reciprocal = 1.f / float(1.f);

        float result = 0.f;

        //    for (uint32_t i = settings_.num_samples; i > 0; --i) {
        float2 const sample = float2(worker.rng().random_float(), worker.rng().random_float());
        //    float2 const sample = sampler_->sample_2D(worker.rng());

        float3 const t = mat_sample.shading_tangent();
        float3 const b = mat_sample.shading_bitangent();
        float3 const n = mat_sample.shading_normal();

        float3 const ws = sample_oriented_hemisphere_cosine(sample, t, b, n);

        occlusion_ray.set_direction(ws);

        if (auto const v = worker.visibility(occlusion_ray, Filter::Undefined); v.valid) {
            result += num_samples_reciprocal;
        }
        //    }

        aov.insert(result, Property::AO);
    }
}

Pool::Pool(uint32_t num_integrators) : num_integrators_(num_integrators) {}

Pool::~Pool() = default;

}  // namespace rendering::integrator::surface
