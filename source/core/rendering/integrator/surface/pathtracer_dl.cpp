#include "pathtracer_dl.hpp"
#include "base/math/vector3.inl"
#include "base/memory/align.hpp"
#include "base/random/generator.inl"
#include "base/spectrum/rgb.hpp"
#include "rendering/integrator/integrator_helper.hpp"
#include "rendering/integrator/surface/surface_integrator.inl"
#include "rendering/rendering_worker.inl"
#include "sampler/sampler_golden_ratio.hpp"
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

namespace rendering::integrator::surface {

using namespace scene;

Pathtracer_DL::Pathtracer_DL(rnd::Generator& rng, Settings const& settings,
                             bool progressive) noexcept
    : Integrator(rng),
      settings_(settings),
      sampler_(rng),
      sampler_pool_(progressive ? nullptr
                                : new sampler::Golden_ratio_pool(2 * Num_dedicated_samplers)) {
    if (sampler_pool_) {
        for (uint32_t i = 0; i < Num_dedicated_samplers; ++i) {
            material_samplers_[i] = sampler_pool_->get(2 * i + 0, rng);
            light_samplers_[i]    = sampler_pool_->get(2 * i + 1, rng);
        }
    } else {
        for (auto& s : material_samplers_) {
            s = &sampler_;
        }

        for (auto& s : light_samplers_) {
            s = &sampler_;
        }
    }
}

Pathtracer_DL::~Pathtracer_DL() noexcept {
    delete sampler_pool_;
}

void Pathtracer_DL::prepare(Scene const& scene, uint32_t num_samples_per_pixel) noexcept {
    sampler_.resize(num_samples_per_pixel, 1, 1, 1);

    for (auto s : material_samplers_) {
        s->resize(num_samples_per_pixel, 1, 1, 1);
    }

    uint32_t const num_lights = scene.num_lights();

    uint32_t const num_light_samples = settings_.num_samples * settings_.light_sampling.num_samples;

    if (Light_sampling::Strategy::Single == settings_.light_sampling.strategy) {
        for (auto s : light_samplers_) {
            s->resize(num_samples_per_pixel, num_light_samples, 1, 2);
        }
    } else {
        for (auto s : light_samplers_) {
            s->resize(num_samples_per_pixel, num_light_samples, num_lights, num_lights);
        }
    }
}

void Pathtracer_DL::start_pixel() noexcept {
    sampler_.start_pixel();

    for (auto s : material_samplers_) {
        s->start_pixel();
    }

    for (auto s : light_samplers_) {
        s->start_pixel();
    }
}

float4 Pathtracer_DL::li(Ray& ray, Intersection& intersection, Worker& worker,
                         Interface_stack const& initial_stack) noexcept {
    worker.reset_interface_stack(initial_stack);

    Filter filter = Filter::Undefined;

    Bxdf_sample sample_result;

    bool primary_ray       = true;
    bool treat_as_singular = true;
    bool evaluate_back     = true;
    bool transparent       = true;

    float3 throughput(1.f);
    float3 result(0.f);

    for (;;) {
        float3 const wo = -ray.direction;

        bool const avoid_caustics = settings_.avoid_caustics && !primary_ray &&
                                    worker.interface_stack().top_is_vacuum_or_not_scattering(
                                        worker);

        auto& material_sample = intersection.sample(wo, ray, filter, avoid_caustics, sampler_,
                                                    worker);

        bool const same_side = material_sample.same_hemisphere(wo);

        if (treat_as_singular & same_side) {
            result += throughput * material_sample.radiance();
        }

        if (material_sample.is_pure_emissive()) {
            transparent &= !worker.scene().prop(intersection.prop)->visible_in_camera() &
                           (ray.max_t >= scene::Ray_max_t);
            break;
        }

        evaluate_back = material_sample.evaluates_back(evaluate_back, same_side);

        result += throughput *
                  direct_light(ray, intersection, material_sample, evaluate_back, filter, worker);

        SOFT_ASSERT(all_finite_and_positive(result));

        material_sample.sample(material_sampler(ray.depth), sample_result);
        if (0.f == sample_result.pdf) {
            break;
        }

        if (sample_result.type.is(Bxdf_type::Caustic)) {
            if (avoid_caustics) {
                break;
            }

            treat_as_singular = sample_result.type.is(Bxdf_type::Specular);
        } else if (sample_result.type.no(Bxdf_type::Straight)) {
            primary_ray       = false;
            filter            = Filter::Nearest;
            treat_as_singular = false;
        }

        if (0.f == ray.wavelength) {
            ray.wavelength = sample_result.wavelength;
        }

        throughput *= sample_result.reflection / sample_result.pdf;

        if (sample_result.type.is(Bxdf_type::Straight)) {
            ray.min_t = scene::offset_f(ray.max_t);
        } else {
            ray.origin = material_sample.offset_p(intersection.geo.p, sample_result.wi);
            ray.min_t  = 0.f;

            ray.set_direction(sample_result.wi);

            transparent = false;
        }

        if (material_sample.ior_greater_one()) {
            ++ray.depth;
        }

        ray.max_t = scene::Ray_max_t;

        if (sample_result.type.is(Bxdf_type::Transmission)) {
            worker.interface_change(sample_result.wi, intersection);
        }

        if (!worker.interface_stack().empty()) {
            float3     vli, vtr;
            auto const hit = worker.volume(ray, intersection, filter, vli, vtr);

            result += throughput * vli;

            throughput *= vtr;

            if (Event::Abort == hit || Event::Absorb == hit) {
                break;
            }
        } else if (!worker.intersect_and_resolve_mask(ray, intersection, filter)) {
            break;
        }

        if (ray.depth >= settings_.max_bounces) {
            break;
        }

        if (ray.depth > settings_.min_bounces) {
            if (russian_roulette(throughput, sampler_.generate_sample_1D())) {
                break;
            }
        }
    }

    return compose_alpha(result, throughput, transparent);
}

float3 Pathtracer_DL::direct_light(Ray const& ray, Intersection const& intersection,
                                   Material_sample const& material_sample, bool evaluate_back,
                                   Filter filter, Worker& worker) noexcept {
    float3 result(0.f);

    if (!material_sample.ior_greater_one()) {
        return result;
    }

    uint32_t const num_samples = settings_.light_sampling.num_samples;

    float3 const p = material_sample.offset_p(intersection.geo.p);

    float3 const n = material_sample.geometric_normal();

    bool const is_translucent = material_sample.is_translucent();

    Ray shadow_ray;
    shadow_ray.origin     = p;
    shadow_ray.min_t      = 0.f;
    shadow_ray.depth      = ray.depth;
    shadow_ray.time       = ray.time;
    shadow_ray.wavelength = ray.wavelength;

    auto& sampler = light_sampler(ray.depth);

    if (Light_sampling::Strategy::Single == settings_.light_sampling.strategy) {
        for (uint32_t i = num_samples; i > 0; --i) {
            float const select = sampler.generate_sample_1D(1);

            // auto const light = worker.scene().random_light(select);
            auto const light = worker.scene().random_light(p, n, is_translucent, select);

            shape::Sample_to light_sample;
            if (!light.ref.sample(p, n, ray.time, is_translucent, sampler, 0, worker,
                                  light_sample)) {
                continue;
            }

            shadow_ray.set_direction(light_sample.wi);
            shadow_ray.max_t = light_sample.t;

            float3 tr;
            if (!worker.transmitted(shadow_ray, material_sample.wo(), intersection, filter, tr)) {
                continue;
            }

            auto const bxdf = material_sample.evaluate_f(light_sample.wi, evaluate_back);

            float3 const radiance = light.ref.evaluate(light_sample, Filter::Nearest, worker);

            float const weight = 1.f / (light.pdf * light_sample.pdf);

            result += weight * (tr * radiance * bxdf.reflection);
        }

        return result / float(num_samples);
    } else {
        for (uint32_t l = 0, len = worker.scene().num_lights(); l < len; ++l) {
            auto const& light = worker.scene().light(l);

            shape::Sample_to light_sample;
            if (!light.sample(p, n, ray.time, is_translucent, sampler, l, worker, light_sample)) {
                continue;
            }

            shadow_ray.set_direction(light_sample.wi);
            shadow_ray.max_t = light_sample.t;

            float3 tr;
            if (!worker.transmitted(shadow_ray, material_sample.wo(), intersection, filter, tr)) {
                continue;
            }

            auto const bxdf = material_sample.evaluate_f(light_sample.wi, evaluate_back);

            float3 const radiance = light.evaluate(light_sample, Filter::Nearest, worker);

            float const weight = 1.f / (light_sample.pdf);

            result += weight * (tr * radiance * bxdf.reflection);
        }

        return result / float(num_samples);
    }
}

sampler::Sampler& Pathtracer_DL::material_sampler(uint32_t bounce) noexcept {
    if (Num_dedicated_samplers > bounce) {
        return *material_samplers_[bounce];
    }

    return sampler_;
}

sampler::Sampler& Pathtracer_DL::light_sampler(uint32_t bounce) noexcept {
    if (Num_dedicated_samplers > bounce) {
        return *light_samplers_[bounce];
    }

    return sampler_;
}

Pathtracer_DL_pool::Pathtracer_DL_pool(uint32_t num_integrators, bool progressive,
                                       uint32_t num_samples, uint32_t min_bounces,
                                       uint32_t max_bounces, Light_sampling light_sampling,
                                       bool enable_caustics) noexcept
    : Typed_pool<Pathtracer_DL>(num_integrators),
      settings_{
          num_samples, min_bounces, max_bounces, light_sampling, !enable_caustics,
      },
      progressive_(progressive) {}

Integrator* Pathtracer_DL_pool::get(uint32_t id, rnd::Generator& rng) const noexcept {
    if (uint32_t const zero = 0;
        0 == std::memcmp(&zero, reinterpret_cast<void*>(&integrators_[id]), 4)) {
        return new (&integrators_[id]) Pathtracer_DL(rng, settings_, progressive_);
    }

    return &integrators_[id];
}

}  // namespace rendering::integrator::surface
