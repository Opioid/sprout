#include "pathtracer_mis.hpp"
#include "base/math/vector4.inl"
#include "base/memory/align.hpp"
#include "base/random/generator.inl"
#include "base/spectrum/rgb.hpp"
#include "rendering/integrator/integrator_helper.hpp"
#include "rendering/rendering_worker.hpp"
#include "scene/light/light.hpp"
#include "scene/material/bxdf.hpp"
#include "scene/material/material.hpp"
#include "scene/material/material_sample.inl"
#include "scene/prop/interface_stack.inl"
#include "scene/prop/prop_intersection.inl"
#include "scene/scene.hpp"
#include "scene/scene_constants.hpp"
#include "scene/scene_ray.inl"
#include "scene/shape/shape_sample.hpp"

#include "base/debug/assert.hpp"

namespace rendering::integrator::surface {

using namespace scene;

Pathtracer_MIS::Pathtracer_MIS(rnd::Generator& rng, take::Settings const& take_settings,
                               Settings const& settings) noexcept
    : Integrator(rng, take_settings),
      settings_(settings),
      sampler_(rng),
      material_samplers_{rng, rng, rng},
      light_samplers_{rng, rng, rng} {}

Pathtracer_MIS::~Pathtracer_MIS() {}

void Pathtracer_MIS::prepare(Scene const& scene, uint32_t num_samples_per_pixel) noexcept {
    uint32_t const num_lights = static_cast<uint32_t>(scene.lights().size());

    sampler_.resize(num_samples_per_pixel, settings_.num_samples, 1, 1);

    for (auto& s : material_samplers_) {
        s.resize(num_samples_per_pixel, settings_.num_samples, 1, 1);
    }

    uint32_t const num_light_samples = settings_.num_samples * settings_.light_sampling.num_samples;

    if (Light_sampling::Strategy::Single == settings_.light_sampling.strategy) {
        for (auto& s : light_samplers_) {
            s.resize(num_samples_per_pixel, num_light_samples, 1, 2);
        }
    } else {
        for (auto& s : light_samplers_) {
            s.resize(num_samples_per_pixel, num_light_samples, num_lights, num_lights);
        }
    }
}

void Pathtracer_MIS::start_pixel() noexcept {
    sampler_.start_pixel();

    for (auto& s : material_samplers_) {
        s.start_pixel();
    }

    for (auto& s : light_samplers_) {
        s.start_pixel();
    }
}

float4 Pathtracer_MIS::li(Ray& ray, Intersection& intersection, Worker& worker,
                          Interface_stack const& initial_stack) noexcept {
    float4 li(0.f);
    float3 photon_li(0.f);

    bool split_photon = false;

    for (uint32_t i = settings_.num_samples; i > 0; --i) {
        worker.reset_interface_stack(initial_stack);

        Ray split_ray = ray;

        Intersection split_intersection = intersection;

        bool const integrate_photons = settings_.num_samples == i;

        Result const result = integrate(split_ray, split_intersection, worker, integrate_photons);

        li += result.li;

        photon_li += result.photon_li;

        split_photon |= result.split_photon;
    }

    float const num_samples_reciprocal = 1.f / static_cast<float>(settings_.num_samples);

    if (split_photon) {
        return num_samples_reciprocal * float4(li.xyz() + photon_li, li[3]);
    }

    return num_samples_reciprocal * li + float4(photon_li, 0.f);
}

size_t Pathtracer_MIS::num_bytes() const noexcept {
    size_t sampler_bytes = 0;

    for (auto const& s : material_samplers_) {
        sampler_bytes += s.num_bytes();
    }

    for (auto const& s : light_samplers_) {
        sampler_bytes += s.num_bytes();
    }

    return sizeof(*this) + sampler_.num_bytes() + sampler_bytes;
}

Pathtracer_MIS::Result Pathtracer_MIS::integrate(Ray& ray, Intersection& intersection,
                                                 Worker& worker, bool integrate_photons) noexcept {
    uint32_t const max_bounces = settings_.max_bounces;

    Filter filter = Filter::Undefined;

    Bxdf_sample sample_result;

    bool primary_ray       = true;
    bool treat_as_singular = true;
    bool is_translucent    = false;
    bool evaluate_back     = true;
    bool split_photon      = false;
    bool transparent       = true;

    float3 throughput(1.f);
    float3 result_li(0.f);
    float3 photon_li(0.f);

    for (uint32_t i = ray.depth;; ++i) {
        float3 const wo = -ray.direction;

        bool const avoid_caustics = settings_.avoid_caustics && !primary_ray &&
                                    worker.interface_stack().top_is_vacuum_or_not_scattering();

        auto const& material_sample = intersection.sample(wo, ray, filter, avoid_caustics, sampler_,
                                                          worker);

        bool const same_side = material_sample.same_hemisphere(wo);

        // Only check direct eye-light connections for the very first hit.
        // Subsequent hits are handled by the MIS scheme.
        if (0 == i && same_side) {
            result_li += material_sample.radiance();
        }

        if (material_sample.is_pure_emissive()) {
            transparent = false;
            break;
        }

        evaluate_back = material_sample.do_evaluate_back(evaluate_back, same_side);

        result_li += throughput * sample_lights(ray, intersection, material_sample, evaluate_back,
                                                filter, worker);

        SOFT_ASSERT(all_finite_and_positive(result_li));

        float const previous_bxdf_pdf = sample_result.pdf;

        // Material BSDF importance sample
        material_sample.sample(material_sampler(ray.depth), sample_result);
        if (0.f == sample_result.pdf) {
            break;
        }

        if (sample_result.type.test(Bxdf_type::Caustic)) {
            if (avoid_caustics) {
                break;
            }

            treat_as_singular = sample_result.type.test(Bxdf_type::Specular);
        } else if (sample_result.type.test_not(Bxdf_type::Pass_through)) {
            treat_as_singular = false;

            if (primary_ray) {
                primary_ray = false;
                filter      = Filter::Nearest;

                if (integrate_photons || 0 != ray.depth) {
                    photon_li    = throughput * worker.photon_li(intersection, material_sample);
                    split_photon = 0 != ray.depth;
                }
            }
        }

        if (0.f == ray.wavelength) {
            ray.wavelength = sample_result.wavelength;
        }

        if (material_sample.ior_greater_one()) {
            transparent = false;

            throughput *= sample_result.reflection / sample_result.pdf;

            ray.origin = material_sample.offset_p(intersection.geo.p, sample_result.wi);
            ray.min_t  = 0.f;
            ray.set_direction(sample_result.wi);
            ++ray.depth;
        } else {
            ray.min_t = scene::offset_f(ray.max_t);
        }

        ray.max_t = scene::Ray_max_t;

        if (sample_result.type.test(Bxdf_type::Transmission)) {
            worker.interface_change(sample_result.wi, intersection);
        }

        if (!worker.interface_stack().empty()) {
            float3     vli, vtr;
            auto const hit = worker.volume(ray, intersection, filter, vli, vtr);

            if (Event::Absorb == hit) {
                if (0 == ray.depth) {
                    // This is the direct eye-light connection for the volume case.
                    result_li += vli;
                } else {
                    result_li += throughput *
                                 evaluate_light_volume(vli, ray, intersection, previous_bxdf_pdf,
                                                       treat_as_singular, is_translucent, worker);
                }

                SOFT_ASSERT(all_finite_and_positive(result_li));

                break;
            }

            throughput *= vtr;

            if (Event::Abort == hit) {
                break;
            }
        } else if (!worker.intersect_and_resolve_mask(ray, intersection, filter)) {
            break;
        }

        SOFT_ASSERT(all_finite(result_li));

        if (!material_sample.ior_greater_one() && !treat_as_singular) {
            sample_result.pdf = previous_bxdf_pdf;
        } else {
            is_translucent = material_sample.is_translucent();
        }

        if (evaluate_back || treat_as_singular) {
            bool         pure_emissive;
            float3 const radiance = evaluate_light(ray, intersection, sample_result,
                                                   treat_as_singular, is_translucent, filter,
                                                   worker, pure_emissive);

            result_li += throughput * radiance;

            if (pure_emissive) {
                transparent = false;
                break;
            }
        }

        if (ray.depth >= max_bounces) {
            break;
        }

        if (ray.depth > settings_.min_bounces) {
            if (russian_roulette(throughput, sampler_.generate_sample_1D())) {
                break;
            }
        }
    }

    return Result{compose_alpha(result_li, throughput, transparent), photon_li, split_photon};
}

float3 Pathtracer_MIS::sample_lights(Ray const& ray, Intersection& intersection,
                                     Material_sample const& material_sample, bool evaluate_back,
                                     Filter filter, Worker& worker) noexcept {
    float3 result(0.f);

    if (!material_sample.ior_greater_one()) {
        return result;
    }

    uint32_t const num_samples = settings_.light_sampling.num_samples;

    float const num_light_samples_reciprocal = 1.f / static_cast<float>(num_samples);

    float3 const p = material_sample.offset_p(intersection.geo.p);

    if (Light_sampling::Strategy::Single == settings_.light_sampling.strategy) {
        for (uint32_t i = num_samples; i > 0; --i) {
            float const select = light_sampler(ray.depth).generate_sample_1D(1);

            auto const light = worker.scene().random_light(select);

            float3 const el = evaluate_light(light.ref, light.pdf, ray, p, 0, evaluate_back,
                                             intersection, material_sample, filter, worker);

            result += num_light_samples_reciprocal * el;
        }
    } else {
        auto const& lights = worker.scene().lights();
        for (uint32_t l = 0, len = static_cast<uint32_t>(lights.size()); l < len; ++l) {
            auto const& light = *lights[l];
            for (uint32_t i = num_samples; i > 0; --i) {
                float3 const el = evaluate_light(light, 1.f, ray, p, l, evaluate_back, intersection,
                                                 material_sample, filter, worker);

                result += num_light_samples_reciprocal * el;
            }
        }
    }

    return result;
}

float3 Pathtracer_MIS::evaluate_light(Light const& light, float light_weight, Ray const& history,
                                      float3 const& p, uint32_t sampler_dimension,
                                      bool evaluate_back, Intersection const& intersection,
                                      Material_sample const& material_sample, Filter filter,
                                      Worker& worker) noexcept {
    // Light source importance sample
    shape::Sample_to light_sample;
    if (!light.sample(p, material_sample.geometric_normal(), history.time,
                      material_sample.is_translucent(), light_sampler(history.depth),
                      sampler_dimension, worker, light_sample)) {
        return float3(0.f);
    }

    Ray shadow_ray(p, light_sample.wi, 0.f, light_sample.t, history.depth, history.time,
                   history.wavelength);

    float3     tv;
    auto const visibility = worker.transmitted_visibility(shadow_ray, material_sample.wo(),
                                                          intersection, filter, tv);
    if (Visibility::None == visibility) {
        return float3(0.f);
    }

    SOFT_ASSERT(all_finite(tv));

    auto const bxdf = material_sample.evaluate_f(light_sample.wi, evaluate_back);

    float3 const radiance = light.evaluate(light_sample, Filter::Nearest, worker);

    float const light_pdf = light_sample.pdf * light_weight;

    bool const mis = evaluate_back && Visibility::Complete == visibility;

    float const weight = mis ? power_heuristic(light_pdf, bxdf.pdf) : 1.f;

    return (weight / light_pdf) * (tv * radiance * bxdf.reflection);
}

float3 Pathtracer_MIS::evaluate_light(Ray const& ray, Intersection const& intersection,
                                      Bxdf_sample sample_result, bool treat_as_singular,
                                      bool is_translucent, Filter filter, Worker& worker,
                                      bool& pure_emissive) noexcept {
    uint32_t const light_id = intersection.light_id();
    if (!Light::is_area_light(light_id)) {
        pure_emissive = false;
        return float3(0.f);
    }

    float light_pdf = 0.f;

    if (!treat_as_singular) {
        bool const calculate_pdf = Light_sampling::Strategy::Single ==
                                   settings_.light_sampling.strategy;

        auto const light = worker.scene().light(light_id, calculate_pdf);

        float const ls_pdf = light.ref.pdf(ray, intersection.geo, is_translucent, Filter::Nearest,
                                           worker);

        if (0.f == ls_pdf) {
            pure_emissive = true;
            return float3(0.f);
        }

        light_pdf = ls_pdf * light.pdf;
    }

    float3 const wo = -sample_result.wi;

    // This will invalidate the contents of previous material samples.
    auto const& light_material_sample = intersection.sample(wo, ray, filter, false, sampler_,
                                                            worker);

    pure_emissive = light_material_sample.is_pure_emissive();

    if (!light_material_sample.same_hemisphere(wo)) {
        return float3(0.f);
    }

    float3 const ls_energy = light_material_sample.radiance();

    float const weight = power_heuristic(sample_result.pdf, light_pdf);

    float3 const radiance = weight * ls_energy;

    SOFT_ASSERT(all_finite_and_positive(radiance));

    return radiance;
}

float3 Pathtracer_MIS::evaluate_light_volume(float3 const& vli, Ray const& ray,
                                             Intersection const& intersection, float bxdf_pdf,
                                             bool treat_as_singular, bool is_translucent,
                                             Worker& worker) const noexcept {
    uint32_t const light_id = intersection.light_id();
    if (!Light::is_light(light_id)) {
        return float3(0.f);
    }

    float light_pdf = 0.f;

    if (!treat_as_singular) {
        bool const calculate_pdf = Light_sampling::Strategy::Single ==
                                   settings_.light_sampling.strategy;

        auto const light = worker.scene().light(light_id, calculate_pdf);

        float const ls_pdf = light.ref.pdf(ray, intersection.geo, is_translucent, Filter::Nearest,
                                           worker);

        if (0.f == ls_pdf) {
            return float3(0.f);
        }

        light_pdf = ls_pdf * light.pdf;
    }

    float const weight = power_heuristic(bxdf_pdf, light_pdf);

    return weight * vli;
}

sampler::Sampler& Pathtracer_MIS::material_sampler(uint32_t bounce) noexcept {
    if (Num_material_samplers > bounce) {
        return material_samplers_[bounce];
    }

    return sampler_;
}

sampler::Sampler& Pathtracer_MIS::light_sampler(uint32_t bounce) noexcept {
    if (Num_light_samplers > bounce) {
        return light_samplers_[bounce];
    }

    return sampler_;
}

Pathtracer_MIS_factory::Pathtracer_MIS_factory(take::Settings const& take_settings,
                                               uint32_t num_integrators, uint32_t num_samples,
                                               uint32_t min_bounces, uint32_t max_bounces,
                                               Light_sampling light_sampling,
                                               bool           enable_caustics) noexcept
    : Factory(take_settings),
      integrators_(memory::allocate_aligned<Pathtracer_MIS>(num_integrators)),
      settings_{num_samples, min_bounces, max_bounces, light_sampling, !enable_caustics} {}

Pathtracer_MIS_factory::~Pathtracer_MIS_factory() noexcept {
    memory::free_aligned(integrators_);
}

Integrator* Pathtracer_MIS_factory::create(uint32_t id, rnd::Generator& rng) const noexcept {
    return new (&integrators_[id]) Pathtracer_MIS(rng, take_settings_, settings_);
}

}  // namespace rendering::integrator::surface
