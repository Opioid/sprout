#include "pathtracer_mis.hpp"
#include "base/math/vector4.inl"
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

#include "base/debug/assert.hpp"

namespace rendering::integrator::surface {

using namespace scene;
using namespace scene::shape;

Pathtracer_MIS::Pathtracer_MIS(rnd::Generator& rng, Settings const& settings, bool progressive)
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

Pathtracer_MIS::~Pathtracer_MIS() {
    delete sampler_pool_;
}

void Pathtracer_MIS::prepare(Scene const& scene, uint32_t num_samples_per_pixel) {
    sampler_.resize(num_samples_per_pixel, settings_.num_samples, 1, 1);

    for (auto s : material_samplers_) {
        s->resize(num_samples_per_pixel, settings_.num_samples, 1, 1);
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

void Pathtracer_MIS::start_pixel() {
    sampler_.start_pixel();

    for (auto s : material_samplers_) {
        s->start_pixel();
    }

    for (auto s : light_samplers_) {
        s->start_pixel();
    }
}

float4 Pathtracer_MIS::li(Ray& ray, Intersection& intersection, Worker& worker,
                          Interface_stack const& initial_stack) {
    float4 li(0.f);
    float3 photon_li(0.f);

    bool split_photon = false;

    for (uint32_t i = settings_.num_samples; i > 0; --i) {
        worker.reset_interface_stack(initial_stack);

        Ray split_ray = ray;

        Intersection split_intersection = intersection;

        bool const integrate_photons = (settings_.num_samples == i) &
                                       settings_.photons_not_only_through_specular;

        Result const result = integrate(split_ray, split_intersection, worker, integrate_photons);

        li += result.li;

        photon_li += result.photon_li;

        split_photon |= result.split_photon;
    }

    float const num_samples_reciprocal = 1.f / float(settings_.num_samples);

    if (split_photon) {
        return num_samples_reciprocal * float4(li.xyz() + photon_li, li[3]);
    }

    return num_samples_reciprocal * li + float4(photon_li, 0.f);
}

Pathtracer_MIS::Result Pathtracer_MIS::integrate(Ray& ray, Intersection& intersection,
                                                 Worker& worker, bool integrate_photons) {
    uint32_t const max_bounces = settings_.max_bounces;

    Filter filter = Filter::Undefined;

    Bxdf_sample sample_result;

    Path_state state;
    state.set(State::Primary_ray);
    state.set(State::Treat_as_singular);
    state.set(State::Transparent);

    float3 throughput(1.f);
    float3 result_li(0.f);
    float3 photon_li(0.f);
    float3 geo_n(0.f);

    for (uint32_t i = ray.depth;; ++i) {
        float3 const wo = -ray.direction;

        bool const avoid_caustics = settings_.avoid_caustics & state.no(State::Primary_ray);

        bool const straight_border = state.is(State::From_subsurface);

        auto const& material_sample = worker.sample_material(
            ray, wo, intersection, filter, avoid_caustics, straight_border, sampler_);

        bool const same_side = material_sample.same_hemisphere(wo);

        // Only check direct eye-light connections for the very first hit.
        // Subsequent hits are handled by MIS.
        if ((0 == i) & same_side) {
            result_li += material_sample.radiance();
        }

        if (material_sample.is_pure_emissive()) {
            state.unset(State::Transparent);
            break;
        }

        if (ray.depth < max_bounces) {
            result_li += throughput *
                         sample_lights(ray, intersection, material_sample, filter, worker);

            SOFT_ASSERT(all_finite_and_positive(result_li));
        }

        float const previous_bxdf_pdf = sample_result.pdf;

        // Material BSDF importance sample
        material_sample.sample(material_sampler(ray.depth), sample_result);
        if (0.f == sample_result.pdf) {
            break;
        }

        if (sample_result.type.is(Bxdf_type::Caustic)) {
            if (avoid_caustics) {
                break;
            }

            state.set(State::Treat_as_singular, sample_result.type.is(Bxdf_type::Specular));
        } else if (sample_result.type.no(Bxdf_type::Straight)) {
            state.unset(State::Treat_as_singular);

            if (state.is(State::Primary_ray)) {
                state.unset(State::Primary_ray);
                filter = Filter::Nearest;

                if (integrate_photons | (0 != ray.depth)) {
                    photon_li = throughput * worker.photon_li(intersection, material_sample);
                    state.set(State::Split_photon, 0 != ray.depth);
                }
            }
        }

        if (0.f == ray.wavelength) {
            ray.wavelength = sample_result.wavelength;
        }

        throughput *= sample_result.reflection / sample_result.pdf;

        if (sample_result.type.is(Bxdf_type::Straight)) {
            ray.min_t() = offset_f(ray.max_t());
        } else {
            ray.origin  = material_sample.offset_p(intersection.geo.p, sample_result.wi);
            // ray.min_t() = 0.f;

            ray.set_direction(sample_result.wi);

            state.unset(State::Transparent);
            state.unset(State::From_subsurface);
        }

        if (material_sample.ior_greater_one()) {
            ++ray.depth;
        }

        ray.max_t() = Ray_max_t;

        if (sample_result.type.is(Bxdf_type::Transmission)) {
            worker.interface_change(sample_result.wi, intersection);
        }

        state.or_set(State::From_subsurface, intersection.subsurface);

        if (!worker.interface_stack().empty()) {
            float3     vli;
            float3     vtr;
            auto const hit = worker.volume(ray, intersection, filter, vli, vtr);

            if (Event::Absorb == hit) {
                if (0 == ray.depth) {
                    // This is the direct eye-light connection for the volume case.
                    result_li += vli;
                } else {
                    result_li += throughput * connect_light_volume(vli, ray, intersection,
                                                                   previous_bxdf_pdf, state,
                                                                   worker);
                }

                SOFT_ASSERT(all_finite_and_positive(result_li));

                break;
            }

            // This is only needed for Tracking_single at the moment...
            result_li += throughput * vli;

            throughput *= vtr;

            if (Event::Abort == hit) {
                break;
            }

            if ((Event::Scatter == hit) & (ray.depth >= max_bounces)) {
                break;
            }
        } else if (!worker.intersect_and_resolve_mask(ray, intersection, filter)) {
            break;
        }

        SOFT_ASSERT(all_finite(result_li));

        if (sample_result.type.is(Bxdf_type::Straight) & state.no(State::Treat_as_singular)) {
            sample_result.pdf = previous_bxdf_pdf;
        } else {
            state.set(State::Is_translucent, material_sample.is_translucent());
            geo_n = material_sample.geometric_normal();
        }

        bool         pure_emissive;
        float3 const radiance = connect_light(ray, geo_n, intersection, sample_result, state,
                                              filter, worker, pure_emissive);

        result_li += throughput * radiance;

        if (pure_emissive) {
            state.and_set(State::Transparent,
                          (!intersection.visible_in_camera(worker)) & (ray.max_t() >= Ray_max_t));
            break;
        }

        if ((ray.depth >= max_bounces) & worker.interface_stack().empty_or_scattering(worker)) {
            break;
        }

        if (ray.depth >= settings_.min_bounces) {
            if (russian_roulette(throughput, sampler_.generate_sample_1D())) {
                break;
            }
        }
    }

    return Result{compose_alpha(result_li, throughput, state.is(State::Transparent)), photon_li,
                  state.is(State::Split_photon)};
}

float3 Pathtracer_MIS::sample_lights(Ray const& ray, Intersection& intersection,
                                     Material_sample const& material_sample, Filter filter,
                                     Worker& worker) {
    float3 result(0.f);

    if (!material_sample.ior_greater_one()) {
        return result;
    }

    uint32_t const num_samples = settings_.light_sampling.num_samples;

    float const num_samples_reciprocal = 1.f / float(num_samples);

    float3 const p = intersection.geo.p;

    if (Light_sampling::Strategy::Single == settings_.light_sampling.strategy) {
        float3 const n = material_sample.geometric_normal();

        bool const is_translucent = material_sample.is_translucent();

        for (uint32_t i = num_samples; i > 0; --i) {
            float const select = light_sampler(ray.depth).generate_sample_1D(1);

            // auto const light = worker.scene().random_light(select);
            auto const light = worker.scene().random_light(p, n, is_translucent, select);

            float3 const el = evaluate_light(light.ref, light.pdf, ray, p, 0, intersection,
                                             material_sample, filter, worker);

            result += num_samples_reciprocal * el;
        }
    } else {
        for (uint32_t l = 0, len = worker.scene().num_lights(); l < len; ++l) {
            auto const& light = worker.scene().light(l);
            for (uint32_t i = num_samples; i > 0; --i) {
                float3 const el = evaluate_light(light, 1.f, ray, p, l, intersection,
                                                 material_sample, filter, worker);

                result += num_samples_reciprocal * el;
            }
        }
    }

    return result;
}

float3 Pathtracer_MIS::evaluate_light(Light const& light, float light_weight, Ray const& history,
                                      float3 const& p, uint32_t sampler_dimension,
                                      Intersection const&    intersection,
                                      Material_sample const& material_sample, Filter filter,
                                      Worker& worker) {
    // Light source importance sample
    Sample_to light_sample;
    if (!light.sample(p, material_sample.geometric_normal(), history.time,
                      material_sample.is_translucent(), light_sampler(history.depth),
                      sampler_dimension, worker, light_sample)) {
        return float3(0.f);
    }

    float3 const origin = material_sample.offset_p(p, light_sample.wi);

    Ray shadow_ray(origin, light_sample.wi, 0.f, light_sample.t(), history.depth, history.wavelength,
                   history.time);

    float3 tr;
    if (!worker.transmitted(shadow_ray, material_sample.wo(), intersection, filter, tr)) {
        return float3(0.f);
    }

    SOFT_ASSERT(all_finite(tr));

    auto const bxdf = material_sample.evaluate_f(light_sample.wi);

    float3 const radiance = light.evaluate(light_sample, Filter::Nearest, worker);

    float const light_pdf = light_sample.pdf() * light_weight;

    float const weight = power_heuristic(light_pdf, bxdf.pdf());

    return (weight / light_pdf) * (tr * radiance * bxdf.reflection);
}

float3 Pathtracer_MIS::connect_light(Ray const& ray, float3 const& geo_n,
                                     Intersection const& intersection, Bxdf_sample sample_result,
                                     Path_state state, Filter filter, Worker& worker,
                                     bool& pure_emissive) {
    uint32_t const light_id = intersection.light_id(worker);
    if (!Light::is_area_light(light_id)) {
        pure_emissive = false;
        return float3(0.f);
    }

    float light_pdf = 0.f;

    if (state.no(State::Treat_as_singular)) {
        bool const calculate_pdf = Light_sampling::Strategy::Single ==
                                   settings_.light_sampling.strategy;

        auto const light = worker.scene().light(light_id, ray.origin, geo_n,
                                                state.is(State::Is_translucent), calculate_pdf);

        float const ls_pdf = light.ref.pdf(ray, intersection.geo, state.is(State::Is_translucent),
                                           Filter::Nearest, worker);

        if (0.f == ls_pdf) {
            pure_emissive = true;
            return float3(0.f);
        }

        light_pdf = ls_pdf * light.pdf;
    }

    float3 const wo = -sample_result.wi;

    // This will invalidate the contents of previous material sample.
    auto const& material_sample = intersection.sample(wo, ray, filter, false, sampler_, worker);

    pure_emissive = material_sample.is_pure_emissive();

    if (!material_sample.same_hemisphere(wo)) {
        return float3(0.f);
    }

    float3 const ls_energy = material_sample.radiance();

    float const weight = power_heuristic(sample_result.pdf, light_pdf);

    float3 const radiance = weight * ls_energy;

    SOFT_ASSERT(all_finite_and_positive(radiance));

    return radiance;
}

float3 Pathtracer_MIS::connect_light_volume(float3 const& vli, Ray const& ray,
                                            Intersection const& intersection, float bxdf_pdf,
                                            Path_state state, Worker& worker) const {
    uint32_t const light_id = intersection.light_id(worker);
    if (!Light::is_light(light_id)) {
        return float3(0.f);
    }

    float light_pdf = 0.f;

    if (state.no(State::Treat_as_singular)) {
        bool const calculate_pdf = Light_sampling::Strategy::Single ==
                                   settings_.light_sampling.strategy;

        auto const light = worker.scene().light(light_id, calculate_pdf);

        float const ls_pdf = light.ref.pdf(ray, intersection.geo, state.is(State::Is_translucent),
                                           Filter::Nearest, worker);

        if (0.f == ls_pdf) {
            return float3(0.f);
        }

        light_pdf = ls_pdf * light.pdf;
    }

    float const weight = power_heuristic(bxdf_pdf, light_pdf);

    return weight * vli;
}

sampler::Sampler& Pathtracer_MIS::material_sampler(uint32_t bounce) {
    if (Num_dedicated_samplers > bounce) {
        return *material_samplers_[bounce];
    }

    return sampler_;
}

sampler::Sampler& Pathtracer_MIS::light_sampler(uint32_t bounce) {
    if (Num_dedicated_samplers > bounce) {
        return *light_samplers_[bounce];
    }

    return sampler_;
}

Pathtracer_MIS_pool::Pathtracer_MIS_pool(uint32_t num_integrators, bool progressive,
                                         uint32_t num_samples, uint32_t min_bounces,
                                         uint32_t max_bounces, Light_sampling light_sampling,
                                         bool enable_caustics, bool photons_only_through_specular)
    : Typed_pool<Pathtracer_MIS>(num_integrators),
      settings_{num_samples,    min_bounces,      max_bounces,
                light_sampling, !enable_caustics, !photons_only_through_specular},
      progressive_(progressive) {}

Integrator* Pathtracer_MIS_pool::get(uint32_t id, rnd::Generator& rng) const {
    if (uint32_t const zero = 0;
        0 == std::memcmp(&zero, static_cast<void*>(&integrators_[id]), 4)) {
        return new (&integrators_[id]) Pathtracer_MIS(rng, settings_, progressive_);
    }

    return &integrators_[id];
}

}  // namespace rendering::integrator::surface
