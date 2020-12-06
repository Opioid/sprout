#include "surface_integrator.hpp"
#include "rendering/rendering_worker.inl"
#include "rendering/sensor/aov/value.inl"
#include "scene/material/material_sample.inl"
#include "scene/prop/prop_intersection.hpp"

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
        aov.insert(float3(float(worker.scene().prop_material_id(isec.prop, isec.geo.part)) / 255.f),
                   Property::Material_id);
    }

    if (aov.active(Property::Depth)) {
        aov.insert(float3(ray.max_t()), Property::Depth);
    }
}

Pool::Pool(uint32_t num_integrators) : num_integrators_(num_integrators) {}

Pool::~Pool() = default;

}  // namespace rendering::integrator::surface
