#include "pathtracer_dl.hpp"
#include "base/math/vector3.inl"
#include "base/memory/align.hpp"
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

Pathtracer_DL::Pathtracer_DL(Settings const& settings, bool progressive)
    : settings_(settings),
      sampler_pool_(progressive ? nullptr
                                : new sampler::Golden_ratio_pool(2 * Num_dedicated_samplers)) {
    if (sampler_pool_) {
        for (uint32_t i = 0; i < Num_dedicated_samplers; ++i) {
            material_samplers_[i] = sampler_pool_->get(2 * i + 0);
            light_samplers_[i]    = sampler_pool_->get(2 * i + 1);
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

Pathtracer_DL::~Pathtracer_DL() {
    delete sampler_pool_;
}

void Pathtracer_DL::prepare(Scene const& scene, uint32_t num_samples_per_pixel) {
    sampler_.resize(num_samples_per_pixel, 1, 1, 1);

    for (auto s : material_samplers_) {
        s->resize(num_samples_per_pixel, 1, 1, 1);
    }

    uint32_t const num_lights = scene.num_lights();

    bool const all = Light_sampling::All == settings_.light_sampling;

    uint32_t const max_lights = light::Tree::max_lights(
        num_lights, Light_sampling::Adaptive == settings_.light_sampling);

    uint32_t const nd2 = all ? num_lights : max_lights;
    uint32_t const nd1 = all ? num_lights : max_lights + 1;

    for (auto s : light_samplers_) {
        s->resize(num_samples_per_pixel, 1, nd2, nd1);
    }

    lights_.reserve(max_lights);
}

void Pathtracer_DL::start_pixel(RNG& rng) {
    sampler_.start_pixel(rng);

    for (auto s : material_samplers_) {
        s->start_pixel(rng);
    }

    for (auto s : light_samplers_) {
        s->start_pixel(rng);
    }
}

float4 Pathtracer_DL::li(Ray& ray, Intersection& isec, Worker& worker,
                         Interface_stack const& initial_stack, AOV* aov) {
    worker.reset_interface_stack(initial_stack);

    Filter filter = Filter::Undefined;

    Bxdf_sample sample_result;

    bool primary_ray       = true;
    bool treat_as_singular = true;
    bool transparent       = true;
    bool from_subsurface   = false;

    float3 throughput(1.f);
    float3 result(0.f);
    float3 wo1(0.f);

    float alpha = 0.f;

    for (uint32_t i = 0;; ++i) {
        float3 const wo = -ray.direction;

        bool const avoid_caustics = settings_.avoid_caustics & (!primary_ray);

        auto const& mat_sample = worker.sample_material(ray, wo, wo1, isec, filter, alpha,
                                                        avoid_caustics, from_subsurface, sampler_);

        alpha = mat_sample.alpha();

        wo1 = wo;

        if (mat_sample.same_hemisphere(wo)) {
            if (treat_as_singular) {
                result += throughput * mat_sample.radiance();
            }
        }

        if (aov) {
            common_AOVs(throughput, ray, isec, mat_sample, primary_ray, worker, *aov);
        }

        if (mat_sample.is_pure_emissive()) {
            transparent &= (!isec.visible_in_camera(worker)) & (ray.max_t() >= Ray_max_t);
            break;
        }

        result += throughput * direct_light(ray, isec, mat_sample, filter, worker);

        SOFT_ASSERT(all_finite_and_positive(result));

        mat_sample.sample(material_sampler(ray.depth), worker.rng(), sample_result);
        if (0.f == sample_result.pdf) {
            break;
        }

        if (sample_result.type.is(Bxdf_type::Caustic)) {
            if (avoid_caustics) {
                break;
            }

            treat_as_singular = sample_result.type.is(Bxdf_type::Specular);
        } else if (sample_result.type.no(Bxdf_type::Straight)) {
            filter            = Filter::Nearest;
            primary_ray       = false;
            treat_as_singular = false;
        }

        if (0.f == ray.wavelength) {
            ray.wavelength = sample_result.wavelength;
        }

        throughput *= sample_result.reflection / sample_result.pdf;

        if (sample_result.type.is(Bxdf_type::Straight)) {
            ray.min_t() = offset_f(ray.max_t());

            if (sample_result.type.no(Bxdf_type::Transmission)) {
                ++ray.depth;
            }
        } else {
            ray.origin = mat_sample.offset_p(isec.geo.p, sample_result.wi, isec.subsurface);
            ray.set_direction(sample_result.wi);
            ++ray.depth;

            transparent     = false;
            from_subsurface = false;
        }

        ray.max_t() = Ray_max_t;

        if (sample_result.type.is(Bxdf_type::Transmission)) {
            worker.interface_change(sample_result.wi, isec);
        }

        from_subsurface |= isec.subsurface;

        if (!worker.interface_stack().empty()) {
            float3     vli;
            float3     vtr;
            auto const hit = worker.volume(ray, isec, filter, vli, vtr);

            if (treat_as_singular) {
                result += throughput * vli;
            }

            throughput *= vtr;

            if ((Event::Abort == hit) | (Event::Absorb == hit)) {
                SOFT_ASSERT(all_finite_and_positive(result));
                break;
            }
        } else if (!worker.intersect_and_resolve_mask(ray, isec, filter)) {
            break;
        }

        if (ray.depth >= settings_.max_bounces) {
            break;
        }

        if (ray.depth >= settings_.min_bounces) {
            if (russian_roulette(throughput, sampler_.sample_1D(worker.rng()))) {
                break;
            }
        }
    }

    return compose_alpha(result, throughput, transparent);
}

float3 Pathtracer_DL::direct_light(Ray const& ray, Intersection const& isec,
                                   Material_sample const& mat_sample, Filter filter,
                                   Worker& worker) {
    float3 result(0.f);

    if (!mat_sample.can_evaluate()) {
        return result;
    }

    bool const translucent = mat_sample.is_translucent();

    float3 const p = mat_sample.offset_p(isec.geo.p, isec.subsurface, translucent);

    float3 const n = mat_sample.geometric_normal();

    Ray shadow_ray;
    shadow_ray.origin     = p;
    shadow_ray.depth      = ray.depth;
    shadow_ray.time       = ray.time;
    shadow_ray.wavelength = ray.wavelength;

    auto& sampler = light_sampler(ray.depth);

    auto& rng = worker.rng();

    if (Light_sampling::All == settings_.light_sampling) {
        for (uint32_t l = 0, len = worker.scene().num_lights(); l < len; ++l) {
            auto const& light = worker.scene().light(l);

            Sample_to light_sample;
            if (!light.sample(p, n, ray.time, translucent, sampler, l, worker, light_sample)) {
                continue;
            }

            shadow_ray.set_direction(light_sample.wi);
            shadow_ray.max_t() = light_sample.t();

            float3 tr;
            if (!worker.transmitted(shadow_ray, mat_sample.wo(), isec, filter, tr)) {
                continue;
            }

            auto const bxdf = mat_sample.evaluate_f(light_sample.wi);

            float3 const radiance = light.evaluate(light_sample, Filter::Nearest, worker);

            float const weight = 1.f / (light_sample.pdf());

            result += weight * (tr * radiance * bxdf.reflection);
        }

        return result;
    }

    float const select = sampler.sample_1D(rng, lights_.capacity());

    bool const split = splitting(ray.depth);

    worker.scene().random_light(p, n, translucent, select, split, lights_);

    for (uint32_t l = 0, len = lights_.size(); l < len; ++l) {
        auto const  light     = lights_[l];
        auto const& light_ref = worker.scene().light(light.id);

        Sample_to light_sample;
        if (!light_ref.sample(p, n, ray.time, translucent, sampler, l, worker, light_sample)) {
            continue;
        }

        shadow_ray.set_direction(light_sample.wi);
        shadow_ray.max_t() = light_sample.t();

        float3 tr;
        if (!worker.transmitted(shadow_ray, mat_sample.wo(), isec, filter, tr)) {
            continue;
        }

        auto const bxdf = mat_sample.evaluate_f(light_sample.wi);

        float3 const radiance = light_ref.evaluate(light_sample, Filter::Nearest, worker);

        float const weight = 1.f / (light.pdf * light_sample.pdf());

        result += weight * (tr * radiance * bxdf.reflection);
    }

    return result;
}

sampler::Sampler& Pathtracer_DL::material_sampler(uint32_t bounce) {
    if (Num_dedicated_samplers > bounce) {
        return *material_samplers_[bounce];
    }

    return sampler_;
}

sampler::Sampler& Pathtracer_DL::light_sampler(uint32_t bounce) {
    if (Num_dedicated_samplers > bounce) {
        return *light_samplers_[bounce];
    }

    return sampler_;
}

bool Pathtracer_DL::splitting(uint32_t bounce) const {
    return (Light_sampling::Adaptive == settings_.light_sampling) &
           (bounce < Num_dedicated_samplers);
}

Pathtracer_DL_pool::Pathtracer_DL_pool(uint32_t num_integrators, bool progressive,
                                       uint32_t num_samples, uint32_t min_bounces,
                                       uint32_t max_bounces, Light_sampling light_sampling,
                                       bool enable_caustics)
    : Typed_pool<Pathtracer_DL>(num_integrators),
      settings_{
          num_samples, min_bounces, max_bounces, light_sampling, !enable_caustics,
      },
      progressive_(progressive) {}

Integrator* Pathtracer_DL_pool::get(uint32_t id) const {
    if (uint32_t const zero = 0;
        0 == std::memcmp(&zero, static_cast<void*>(&integrators_[id]), 4)) {
        return new (&integrators_[id]) Pathtracer_DL(settings_, progressive_);
    }

    return &integrators_[id];
}

}  // namespace rendering::integrator::surface
