#include "surface_integrator.hpp"
#include "rendering/rendering_worker.inl"
#include "rendering/sensor/aov/value.inl"
#include "scene/material/material_sample.inl"
#include "scene/prop/prop_intersection.hpp"

namespace rendering::integrator::surface {

Integrator::Integrator() = default;

Integrator::~Integrator() = default;

void Integrator::common_AOVs(Intersection& isec, Material_sample const& mat_sample, Worker& worker,
                             AOV& aov) {
    using Property = sensor::aov::Property;

    aov.insert(mat_sample.albedo(), Property::Albedo);

    if (aov.active(Property::Roughness)) {
        float const a = mat_sample.alpha();
        float const r = std::sqrt(a);
        aov.insert(float3(r), Property::Roughness);
    }

    if (aov.active(Property::Geometric_normal)) {
        aov.insert(abs(0.5f * (mat_sample.geometric_normal() + 1.f)), Property::Geometric_normal);
    }

    if (aov.active(Property::Shading_normal)) {
        aov.insert(abs(0.5f * (mat_sample.shading_normal() + 1.f)), Property::Shading_normal);
    }

    if (aov.active(Property::Material_id)) {
        aov.insert(float3(float(worker.scene().prop_material_id(isec.prop, isec.geo.part)) / 255.f),
                   Property::Material_id);
    }
}

Pool::Pool(uint32_t num_integrators) : num_integrators_(num_integrators) {}

Pool::~Pool() = default;

}  // namespace rendering::integrator::surface
