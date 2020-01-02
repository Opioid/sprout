#include "debug.hpp"
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

Debug::Debug(rnd::Generator& rng, Settings const& settings) noexcept
    : Integrator(rng), settings_(settings), sampler_(rng) {}

void Debug::prepare(scene::Scene const& /*scene*/, uint32_t /*num_samples_per_pixel*/) noexcept {}

void Debug::start_pixel() noexcept {}

float4 Debug::li(Ray& ray, Intersection& intersection, Worker& worker,
                 Interface_stack const& initial_stack) noexcept {
    worker.reset_interface_stack(initial_stack);

    float3 vector;

    switch (settings_.vector) {
        case Settings::Vector::Tangent:
            vector = intersection.geo.t;
            break;
        case Settings::Vector::Bitangent:
            vector = intersection.geo.b;
            break;
        case Settings::Vector::Geometric_normal:
            vector = intersection.geo.geo_n;
            break;
        case Settings::Vector::Shading_normal: {
            float3 const wo = -ray.direction;

            auto& material_sample = intersection.sample(wo, ray, Filter::Undefined, false, sampler_,
                                                        worker);

            if (!material_sample.same_hemisphere(wo)) {
                return float4(0.f, 0.f, 0.f, 1.f);
            }

            vector = material_sample.base_shading_normal();
        } break;
        case Settings::Vector::UV:
            vector = float3(0.5f * intersection.geo.uv, -1.f);
            break;
        default:
            return float4(0.f, 0.f, 0.f, 1.f);
    }

    return float4(0.5f * (vector + float3(1.f)), 1.f);
}

Debug_pool::Debug_pool(uint32_t num_integrators, Debug::Settings::Vector vector) noexcept
    : Typed_pool<Debug>(num_integrators) {
    settings_.vector = vector;
}

Integrator* Debug_pool::create(uint32_t id, rnd::Generator& rng) const noexcept {
    return new (&integrators_[id]) Debug(rng, settings_);
}

}  // namespace rendering::integrator::surface
