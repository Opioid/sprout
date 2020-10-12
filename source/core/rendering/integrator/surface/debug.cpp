#include "debug.hpp"
#include "base/math/vector4.inl"
#include "base/memory/align.hpp"
#include "base/memory/array.inl"
#include "base/random/generator.inl"
#include "rendering/integrator/surface/surface_integrator.inl"
#include "rendering/rendering_worker.hpp"
#include "scene/material/material.hpp"
#include "scene/material/material_sample.inl"
#include "scene/prop/prop_intersection.inl"
#include "scene/scene_ray.inl"

namespace rendering::integrator::surface {

Debug::Debug(Settings const& settings) : settings_(settings) {
    lights_.reserve(Settings::Value::Splitting == settings.value ? scene::light::Tree::Max_lights
                                                                 : 0);
}

void Debug::prepare(scene::Scene const& /*scene*/, uint32_t num_samples_per_pixel) {
    sampler_.resize(num_samples_per_pixel, 1, 1, 1);
}

void Debug::start_pixel(rnd::Generator& /*rng*/) {}

float4 Debug::li(Ray& ray, Intersection& isec, Worker& worker,
                 Interface_stack const& initial_stack) {
    worker.reset_interface_stack(initial_stack);

    float3 vector;

    switch (settings_.value) {
        case Settings::Value::Tangent:
            vector = isec.geo.t;
            break;
        case Settings::Value::Bitangent:
            vector = isec.geo.b;
            break;
        case Settings::Value::Geometric_normal:
            vector = isec.geo.geo_n;
            break;
        case Settings::Value::Shading_normal: {
            float3 const wo = -ray.direction;

            auto const& mat_sample = isec.sample(wo, ray, Filter::Undefined, false, sampler_,
                                                 worker);

            if (!mat_sample.same_hemisphere(wo)) {
                return float4(0.f, 0.f, 0.f, 1.f);
            }

            vector = mat_sample.interpolated_normal();
        } break;
        case Settings::Value::UV:
            vector = float3(0.5f * isec.geo.uv, -1.f);
            break;
        case Settings::Value::Splitting: {
            float3 const wo = -ray.direction;

            auto const& mat_sample = isec.sample(wo, ray, Filter::Undefined, false, sampler_,
                                                 worker);

            float3 const n = mat_sample.interpolated_normal();

            bool const translucent = mat_sample.is_translucent();

            worker.scene().random_light(isec.geo.p, n, translucent, worker.rng().random_float(), true, lights_);

            float const r = float(lights_.size()) / float(scene::light::Tree::Max_lights);

            return float4(r, r, r, 1.f);
        } break;
        default:
            return float4(0.f, 0.f, 0.f, 1.f);
    }

    return float4(abs(0.5f * (vector + float3(1.f))), 1.f);
}

Debug_pool::Debug_pool(uint32_t num_integrators, Debug::Settings::Value vector)
    : Typed_pool<Debug>(num_integrators) {
    settings_.value = vector;
}

Integrator* Debug_pool::get(uint32_t id) const {
    if (uint32_t const zero = 0;
        0 == std::memcmp(&zero, static_cast<void*>(&integrators_[id]), 4)) {
        return new (&integrators_[id]) Debug(settings_);
    }

    return &integrators_[id];
}

}  // namespace rendering::integrator::surface
