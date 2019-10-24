#include "pm.hpp"
#include "base/math/vector3.inl"
#include "base/memory/align.hpp"
#include "base/random/generator.inl"
#include "base/spectrum/rgb.hpp"
#include "rendering/integrator/integrator_helper.hpp"
#include "rendering/rendering_worker.hpp"
#include "scene/light/light.inl"
#include "scene/material/bxdf.hpp"
#include "scene/material/material.hpp"
#include "scene/material/material_sample.inl"
#include "scene/prop/interface_stack.inl"
#include "scene/prop/prop_intersection.inl"
#include "scene/scene.hpp"
#include "scene/scene_constants.hpp"
#include "scene/scene_ray.inl"
#include "scene/shape/shape_sample.hpp"

#define ONLY_CAUSTICS

namespace rendering::integrator::surface {

PM::PM(rnd::Generator& rng, take::Settings const& take_settings, Settings const& settings) noexcept
    : Integrator(rng, take_settings),
      settings_(settings),
      sampler_(rng),
      material_samplers_{rng, rng, rng} {}

PM::~PM() noexcept {}

void PM::prepare(Scene const& /*scene*/, uint32_t num_samples_per_pixel) noexcept {
    sampler_.resize(num_samples_per_pixel, 1, 1, 1);

    for (auto& s : material_samplers_) {
        s.resize(num_samples_per_pixel, 1, 1, 1);
    }
}

void PM::start_pixel() noexcept {
    sampler_.start_pixel();

    for (auto& s : material_samplers_) {
        s.start_pixel();
    }
}

float4 PM::li(Ray& ray, Intersection& intersection, Worker& worker,
              Interface_stack const& initial_stack) noexcept {
    static uint32_t constexpr Max_bounces = 16;

    worker.reset_interface_stack(initial_stack);

    Filter filter = Filter::Undefined;

    Bxdf_sample sample_result;

    float3 throughput(1.f);

    float3 result = float3(0.f);

    bool const avoid_caustics = true;

    for (uint32_t i = ray.depth;; ++i) {
        float3 const wo = -ray.direction;

        auto const& material_sample = intersection.sample(wo, ray, filter, avoid_caustics, sampler_,
                                                          worker);

        if (material_sample.same_hemisphere(wo)) {
#ifndef ONLY_CAUSTICS
            result += throughput * material_sample.radiance();
#endif
        }

        if (material_sample.is_pure_emissive()) {
            break;
        }

        material_sample.sample(material_sampler(ray.depth), sample_result);
        if (0.f == sample_result.pdf) {
            break;
        }

        if (!sample_result.type.is(Bxdf_type::Caustic)) {
            if (ray.depth > 0 || settings_.photons_not_only_through_specular) {
                result += throughput * worker.photon_li(intersection, material_sample);
            }

            break;
        }

        if (ray.depth >= Max_bounces - 1) {
            break;
        }

        if (0.f == ray.wavelength) {
            ray.wavelength = sample_result.wavelength;
        }

        if (material_sample.ior_greater_one()) {
            throughput *= sample_result.reflection / sample_result.pdf;

            ray.origin = material_sample.offset_p(intersection.geo.p, sample_result.wi);
            ray.min_t  = 0.f;
            ray.set_direction(sample_result.wi);
            ++ray.depth;
        } else {
            ray.min_t = scene::offset_f(ray.max_t);
        }

        ray.max_t = scene::Ray_max_t;

        if (sample_result.type.is(Bxdf_type::Transmission)) {
            worker.interface_change(sample_result.wi, intersection);
        }

        if (!worker.interface_stack().empty()) {
            float3     vli, vtr;
            auto const hit = worker.volume(ray, intersection, filter, vli, vtr);

            // result += throughput * vli;
            throughput *= vtr;

            if (Event::Abort == hit) {
                break;
            }
        } else if (!worker.intersect_and_resolve_mask(ray, intersection, filter)) {
            break;
        }
    }

    return float4(result, 1.f);
}

sampler::Sampler& PM::material_sampler(uint32_t bounce) noexcept {
    if (Num_material_samplers > bounce) {
        return material_samplers_[bounce];
    }

    return sampler_;
}

PM_factory::PM_factory(take::Settings const& take_settings, uint32_t num_integrators,
                       uint32_t min_bounces, uint32_t max_bounces,
                       bool photons_only_through_specular) noexcept
    : Factory(take_settings),
      integrators_(memory::allocate_aligned<PM>(num_integrators)),
      settings_{min_bounces, max_bounces, !photons_only_through_specular} {}

PM_factory::~PM_factory() noexcept {
    memory::free_aligned(integrators_);
}

Integrator* PM_factory::create(uint32_t id, rnd::Generator& rng) const noexcept {
    return new (&integrators_[id]) PM(rng, take_settings_, settings_);
}

}  // namespace rendering::integrator::surface
