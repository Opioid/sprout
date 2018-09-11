#include "debug.hpp"
#include "base/math/vector4.inl"
#include "base/memory/align.hpp"
#include "base/random/generator.inl"
#include "rendering/rendering_worker.hpp"
#include "scene/material/material.hpp"
#include "scene/material/material_sample.inl"
#include "scene/prop/prop_intersection.inl"
#include "scene/scene_ray.inl"

namespace rendering::integrator::surface {

Debug::Debug(rnd::Generator& rng, take::Settings const& take_settings,
             Settings const& settings) noexcept
    : Integrator(rng, take_settings), settings_(settings), sampler_(rng) {}

void Debug::prepare(scene::Scene const& /*scene*/, uint32_t /*num_samples_per_pixel*/) noexcept {}

void Debug::resume_pixel(uint32_t /*sample*/, rnd::Generator& /*scramble*/) noexcept {}

float3 Debug::li(Ray& ray, Intersection& intersection, Worker& worker,
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

            auto& material_sample = intersection.sample(wo, ray, Sampler_filter::Undefined, false,
                                                        sampler_, worker);

            if (!material_sample.same_hemisphere(wo)) {
                return float3(0.f);
            }

            vector = material_sample.base_layer().shading_normal();
        } break;
        case Settings::Vector::UV:
            vector = float3(0.5f * intersection.geo.uv, -1.f);
            break;
        default:
            return float3(0.f);
    }

    return float3(0.5f * (vector + float3(1.f)));
}

size_t Debug::num_bytes() const noexcept {
    return sizeof(*this);
}

Debug_factory::Debug_factory(take::Settings const& take_settings, uint32_t num_integrators,
                             Debug::Settings::Vector vector) noexcept
    : Factory(take_settings), integrators_(memory::allocate_aligned<Debug>(num_integrators)) {
    settings_.vector = vector;
}

Debug_factory::~Debug_factory() noexcept {
    memory::free_aligned(integrators_);
}

Integrator* Debug_factory::create(uint32_t id, rnd::Generator& rng) const noexcept {
    return new (&integrators_[id]) Debug(rng, take_settings_, settings_);
}

}  // namespace rendering::integrator::surface
