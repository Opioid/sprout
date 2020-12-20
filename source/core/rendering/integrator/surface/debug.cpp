#include "debug.hpp"
#include "base/math/vector4.inl"
#include "base/memory/align.hpp"
#include "base/memory/array.inl"
#include "base/random/generator.inl"
#include "base/spectrum/mapping.hpp"
#include "base/spectrum/rgb.hpp"
#include "rendering/integrator/surface/surface_integrator.inl"
#include "rendering/rendering_worker.hpp"
#include "rendering/sensor/aov/value.inl"
#include "scene/material/material.hpp"
#include "scene/material/material_sample.inl"
#include "scene/prop/prop_intersection.inl"
#include "scene/scene_ray.inl"

namespace rendering::integrator::surface {

using namespace scene;

Debug::Debug(Settings const& settings) : settings_(settings) {
    lights_.reserve(Settings::Value::Splitting == settings.value ? light::Tree::Max_lights : 0);
}

void Debug::prepare(Scene const& /*scene*/, uint32_t num_samples_per_pixel) {
    sampler_.resize(num_samples_per_pixel, 1, 1);
}

void Debug::start_pixel(rnd::Generator& /*rng*/) {}

float4 Debug::li(Ray& ray, Intersection& isec, Worker& worker, Interface_stack const& initial_stack,
                 AOV* aov) {
    worker.reset_interface_stack(initial_stack);

    float3 const wo = -ray.direction;

    auto const& mat_sample = isec.sample(wo, ray, Filter::Undefined, 0.f, false, sampler_, worker);

    if (aov) {
        common_AOVs(float3(1.f), ray, isec, mat_sample, true, worker, *aov);
    }

    float3 vector;

    switch (settings_.value) {
        case Settings::Value::Albedo:
            return float4(mat_sample.albedo(), 1.f);
        case Settings::Value::Roughness: {
            float const a = mat_sample.alpha();
            float const r = std::sqrt(a);
            return float4(r, r, r, 1.f);
        } break;
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
            if (!mat_sample.same_hemisphere(wo)) {
                return float4(0.f, 0.f, 0.f, 1.f);
            }

            vector = mat_sample.shading_normal();
        } break;
        case Settings::Value::UV:
            return float4(frac(isec.geo.uv[0]), frac(isec.geo.uv[1]), 0.f, 1.f);
        case Settings::Value::Splitting: {
            float3_p n = mat_sample.interpolated_normal();

            bool const translucent = mat_sample.is_translucent();

            worker.scene().random_light(isec.geo.p, n, translucent, 1.f, true, lights_);

            float const r = float(lights_.size()) / float(light::Tree::Max_lights);

            return float4(r, r, r, 1.f);
        } break;
        case Settings::Value::Material_id: {
            uint32_t const mat_id = worker.scene().prop_material_id(isec.prop, isec.geo.part);

            return float4(float(mat_id), 0.f, 0.f, 1.f);
        } break;
        case Settings::Value::Light_id:
            return float4(light_id(ray, isec, worker), 1.f);
        case Settings::Value::Backface: {
            return mat_sample.same_hemisphere(wo) ? float4(0.f, 0.f, 0.f, 1.f) : float4(1.f);
        } break;
        default:
            return float4(0.f, 0.f, 0.f, 1.f);
    }

    return float4(saturate(0.5f * (vector + 1.f)), 1.f);
}

float3 Debug::light_id(Ray& ray, Intersection& isec, Worker& worker) {
    for (uint32_t i = 0; i < 16; ++i) {
        uint32_t const light_id = isec.light_id(worker);
        if (!Light::is_light(light_id)) {
            ray.min_t() = offset_f(ray.max_t());
            ray.max_t() = scene::Ray_max_t;

            if (!worker.intersect(ray, isec)) {
                return float3(1.f);
            }

            continue;
        }

        uint32_t const stripped_id = light::Light::strip_mask(light_id);

        float const s = float(stripped_id) / float(worker.scene().num_lights() - 1);

        return spectrum::gamma_to_linear_sRGB(spectrum::turbo(s));
    }

    return float3(1.f);
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
