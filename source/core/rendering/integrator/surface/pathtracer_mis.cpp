#include "pathtracer_mis.hpp"
#include "base/math/vector4.inl"
#include "base/memory/array.inl"
#include "base/random/generator.inl"
#include "base/spectrum/rgb.hpp"
#include "rendering/integrator/integrator_helper.hpp"
#include "rendering/integrator/surface/surface_integrator.inl"
#include "rendering/rendering_worker.inl"
#include "rendering/sensor/aov/value.inl"
#include "sampler/sampler_golden_ratio.hpp"
#include "scene/light/light.inl"
#include "scene/material/bxdf.hpp"
#include "scene/material/material.inl"
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

Pathtracer_MIS::Pathtracer_MIS(Settings const& settings, uint32_t max_samples_per_pixel,
                               bool progressive)
    : settings_(settings) {
    if (progressive) {
        sampler_pool_ = new sampler::Random_pool(2 * Num_dedicated_samplers);
    } else {
        sampler_pool_ = new sampler::Golden_ratio_pool(2 * Num_dedicated_samplers);
    }

    static uint32_t constexpr Max_lights = light::Tree::Max_lights;

    uint32_t const max_samples = max_samples_per_pixel * settings_.num_samples;

    for (uint32_t i = 0; i < Num_dedicated_samplers; ++i) {
        sampler_pool_->create(2 * i + 0, 2, 1, max_samples);
        sampler_pool_->create(2 * i + 1, Max_lights, Max_lights + 1, max_samples);
    }
}

Pathtracer_MIS::~Pathtracer_MIS() {
    delete sampler_pool_;
}

void Pathtracer_MIS::start_pixel(RNG& rng) {
    sampler_.start_pixel(rng);

    for (uint32_t i = 0; i < 2 * Num_dedicated_samplers; ++i) {
        sampler_pool_->get(i).start_pixel(rng);
    }
}

float4 Pathtracer_MIS::li(Ray& ray, Intersection& isec, Worker& worker,
                          Interface_stack const& initial_stack, AOV* aov) {
    float4 li(0.f);
    float3 photon_li(0.f);

    bool split_photon = false;

    for (uint32_t i = settings_.num_samples; i > 0; --i) {
        worker.reset_interface_stack(initial_stack);

        Ray split_ray = ray;

        Intersection split_isec = isec;

        bool const integrate_photons = (settings_.num_samples == i) &
                                       settings_.photons_not_only_through_specular;

        Result const result = integrate(split_ray, split_isec, worker, integrate_photons, aov);

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

Pathtracer_MIS::Result Pathtracer_MIS::integrate(Ray& ray, Intersection& isec, Worker& worker,
                                                 bool integrate_photons, AOV* aov) {
    uint32_t const max_bounces = settings_.max_bounces;

    Bxdf_sample sample_result;

    Path_state state;
    state.set(State::Primary_ray);
    state.set(State::Treat_as_singular);
    state.set(State::Direct);

    float3 throughput(1.f);
    float3 result_li(0.f);
    float3 photon_li(0.f);
    float3 geo_n(0.f);
    float3 wo1(0.f);

    float alpha = 0.f;

    for (uint32_t i = ray.depth;; ++i) {
        float3 const wo = -ray.direction;

        bool const pr              = state.is(State::Primary_ray);
        bool const avoid_caustics  = settings_.avoid_caustics & !pr;
        bool const straight_border = state.is(State::From_subsurface);

        Filter const filter = ((ray.depth <= 1) | pr) ? Filter::Undefined : Filter::Nearest;

        auto const& mat_sample = worker.sample_material(ray, wo, wo1, isec, filter, alpha,
                                                        avoid_caustics, straight_border, sampler_);

        alpha = mat_sample.alpha();

        wo1 = wo;

        // Only check direct eye-light connections for the very first hit.
        // Subsequent hits are handled by MIS.
        if (0 == i && mat_sample.same_hemisphere(wo)) {
            result_li += mat_sample.radiance();
        }

        if (aov) {
            common_AOVs(throughput, ray, isec, mat_sample, state.is(State::Primary_ray), worker,
                        *aov);
        }

        if (mat_sample.is_pure_emissive()) {
            state.unset(State::Direct);
            break;
        }

        result_li += throughput * sample_lights(ray, isec, mat_sample, filter, worker);

        SOFT_ASSERT(all_finite_and_positive(result_li));

        float effective_bxdf_pdf = sample_result.pdf;

        // Material BSDF importance sample
        mat_sample.sample(material_sampler(ray.depth), worker.rng(), sample_result);
        if (0.f == sample_result.pdf) {
            break;
        }

        if (sample_result.type.is(Bxdf_type::Specular)) {
            if (avoid_caustics) {
                break;
            }

            state.set(State::Treat_as_singular);
        } else if (sample_result.type.no(Bxdf_type::Straight)) {
            state.unset(State::Treat_as_singular);

            effective_bxdf_pdf = sample_result.pdf;

            if (state.is(State::Primary_ray)) {
                state.unset(State::Primary_ray);

                if (bool const indirect = state.no(State::Direct) & (0 != ray.depth);
                    integrate_photons | indirect) {
                    photon_li = throughput * worker.photon_li(isec, mat_sample);
                    state.set(State::Split_photon, indirect);
                }
            }
        }

        if (sample_result.type != Bxdf_type::Straight_transmission) {
            ++ray.depth;
        }

        if (sample_result.type.is(Bxdf_type::Straight)) {
            ray.min_t() = offset_f(ray.max_t());
        } else {
            ray.origin = isec.offset_p(sample_result.wi);
            ray.set_direction(sample_result.wi);

            state.unset(State::Direct);
            state.unset(State::From_subsurface);
        }

        ray.max_t() = Ray_max_t;

        if (0.f == ray.wavelength) {
            ray.wavelength = sample_result.wavelength;
        }

        throughput *= sample_result.reflection / sample_result.pdf;

        if (sample_result.type.is(Bxdf_type::Transmission)) {
            worker.interface_change(sample_result.wi, isec);
        }

        state.or_set(State::From_subsurface, isec.subsurface);

        if (sample_result.type.is(Bxdf_type::Straight) & state.no(State::Treat_as_singular)) {
            sample_result.pdf = effective_bxdf_pdf;
        } else {
            state.set(State::Is_translucent, mat_sample.is_translucent());
            geo_n = mat_sample.geometric_normal();
        }

        if (!worker.interface_stack().empty()) {
            float3     vli;
            float3     vtr;
            auto const hit = worker.volume(ray, isec, filter, vli, vtr);

            if (Event::Absorb == hit) {
                if (0 == ray.depth) {
                    // This is the direct eye-light connection for the volume case.
                    result_li += vli;
                } else {
                    float const w = connect_light_volume(ray, geo_n, isec, effective_bxdf_pdf,
                                                         state, worker);

                    result_li += w * throughput * vli;
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
        } else if (!worker.intersect_and_resolve_mask(ray, isec, filter)) {
            break;
        }

        SOFT_ASSERT(all_finite(result_li));

        bool         pure_emissive;
        float3 const radiance = connect_light(ray, geo_n, isec, sample_result, state, filter,
                                              worker, pure_emissive);

        result_li += throughput * radiance;

        if (pure_emissive) {
            state.and_set(State::Direct,
                          (!isec.visible_in_camera(worker)) & (ray.max_t() >= Ray_max_t));
            break;
        }

        if (ray.depth >= max_bounces) {
            break;
        }

        if (ray.depth >= settings_.min_bounces) {
            if (russian_roulette(throughput, sampler_.sample_1D(worker.rng()))) {
                break;
            }
        }
    }

    return Result{compose_alpha(result_li, throughput, state.is(State::Direct)), photon_li,
                  state.is(State::Split_photon)};
}

float3 Pathtracer_MIS::sample_lights(Ray const& ray, Intersection& isec,
                                     Material_sample const& mat_sample, Filter filter,
                                     Worker& worker) {
    if (!mat_sample.can_evaluate()) {
        return float3(0.f);
    }

    bool const translucent = mat_sample.is_translucent();

    float3 const p = isec.offset_p(translucent);
    float3 const n = mat_sample.geometric_normal();

    auto& rng = worker.rng();

    auto& lights = worker.lights();

    float const select = light_sampler(ray.depth).sample_1D(rng, lights.capacity());

    bool const split = splitting(ray.depth);

    worker.scene().random_light(p, n, translucent, select, split, lights);

    float3 result(0.f);
    for (uint32_t l = 0; auto const light : lights) {
        auto const& light_ref = worker.scene().light(light.offset);

        float3 const el = evaluate_light(light_ref, light.pdf, ray, p, l, isec, mat_sample, filter,
                                         worker);

        result += el;

        ++l;
    }

    return result;
}

float3 Pathtracer_MIS::evaluate_light(Light const& light, float light_weight, Ray const& history,
                                      float3_p p, uint32_t sampler_d, Intersection const& isec,
                                      Material_sample const& mat_sample, Filter filter,
                                      Worker& worker) {
    // Light source importance sample
    Sample_to light_sample;
    if (!light.sample(p, mat_sample.geometric_normal(), history.time, mat_sample.is_translucent(),
                      light_sampler(history.depth), sampler_d, worker, light_sample)) {
        return float3(0.f);
    }

    Ray shadow_ray(p, light_sample.wi, p[3], light_sample.t(), history.depth, history.wavelength,
                   history.time);

    float3 tr;
    if (!worker.transmitted(shadow_ray, mat_sample.wo(), isec, filter, tr)) {
        return float3(0.f);
    }

    SOFT_ASSERT(all_finite(tr));

    auto const bxdf = mat_sample.evaluate(light_sample.wi);

    float3 const radiance = light.evaluate(light_sample, Filter::Nearest, worker);

    float const light_pdf = light_sample.pdf() * light_weight;
    float const weight    = predivided_power_heuristic(light_pdf, bxdf.pdf());

    return weight * (tr * radiance * bxdf.reflection);
}

float3 Pathtracer_MIS::connect_light(Ray const& ray, float3_p geo_n, Intersection const& isec,
                                     Bxdf_sample sample_result, Path_state state, Filter filter,
                                     Worker& worker, bool& pure_emissive) {
    uint32_t const light_id = isec.light_id(worker);
    if (!Light::is_area_light(light_id)) {
        pure_emissive = false;
        return float3(0.f);
    }

    float3 const wo = -sample_result.wi;

    float3 ls_energy;
    if (!isec.evaluate_radiance(wo, filter, worker, ls_energy, pure_emissive)) {
        return float3(0.f);
    }

    if (state.is(State::Treat_as_singular)) {
        return ls_energy;
    }

    bool const translucent = state.is(State::Is_translucent);
    bool const split       = splitting(ray.depth);

    auto const light = worker.scene().light(light_id, ray.origin, geo_n, translucent, split);

    float const ls_pdf = light.ref.pdf(ray, geo_n, isec, translucent, Filter::Nearest, worker);
    float const weight = power_heuristic(sample_result.pdf, ls_pdf * light.pdf);

    SOFT_ASSERT(std::isfinite(weight) && weight >= 0.f);

    return weight * ls_energy;
}

float Pathtracer_MIS::connect_light_volume(Ray const& ray, float3_p geo_n, Intersection const& isec,
                                           float bxdf_pdf, Path_state state, Worker& worker) const {
    uint32_t const light_id = isec.light_id(worker);
    if (!Light::is_light(light_id)) {
        return 0.f;
    }

    if (state.is(State::Treat_as_singular)) {
        return 1.f;
    }

    bool const translucent = state.is(State::Is_translucent);
    bool const split       = splitting(ray.depth);

    auto const light = worker.scene().light(light_id, ray.origin, geo_n, translucent, split);

    float const ls_pdf = light.ref.pdf(ray, geo_n, isec, translucent, Filter::Nearest, worker);

    return power_heuristic(bxdf_pdf, ls_pdf * light.pdf);
}

sampler::Sampler& Pathtracer_MIS::material_sampler(uint32_t bounce) {
    if (Num_dedicated_samplers > bounce) {
        return sampler_pool_->get(2 * bounce + 0);
    }

    return sampler_;
}

sampler::Sampler& Pathtracer_MIS::light_sampler(uint32_t bounce) {
    if (Num_dedicated_samplers > bounce) {
        return sampler_pool_->get(2 * bounce + 1);
    }

    return sampler_;
}

bool Pathtracer_MIS::splitting(uint32_t bounce) const {
    return (Light_sampling::Adaptive == settings_.light_sampling) &
           (bounce < Num_dedicated_samplers);
}

Pathtracer_MIS_pool::Pathtracer_MIS_pool(uint32_t num_integrators, bool progressive,
                                         uint32_t num_samples, uint32_t min_bounces,
                                         uint32_t max_bounces, Light_sampling light_sampling,
                                         bool enable_caustics, bool photons_only_through_specular)
    : Typed_pool<Pathtracer_MIS>(num_integrators),
      settings_{num_samples,    min_bounces,      max_bounces,
                light_sampling, !enable_caustics, !photons_only_through_specular},
      progressive_(progressive) {}

Integrator* Pathtracer_MIS_pool::create(uint32_t id, uint32_t max_samples_per_pixel) const {
    return new (&integrators_[id]) Pathtracer_MIS(settings_, max_samples_per_pixel, progressive_);
}

}  // namespace rendering::integrator::surface
