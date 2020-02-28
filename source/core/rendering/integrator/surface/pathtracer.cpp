#include "pathtracer.hpp"
#include "base/math/vector3.inl"
#include "base/memory/align.hpp"
#include "base/random/generator.inl"
#include "base/spectrum/rgb.hpp"
#include "rendering/integrator/integrator_helper.hpp"
#include "rendering/integrator/surface/surface_integrator.inl"
#include "rendering/rendering_worker.inl"
#include "sampler/sampler_golden_ratio.hpp"
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

// #define ONLY_CAUSTICS

using namespace scene;

namespace rendering::integrator::surface {

Pathtracer::Pathtracer(rnd::Generator& rng, Settings const& settings, bool progressive)
    : Integrator(rng),
      settings_(settings),
      sampler_(rng),
      sampler_pool_(progressive ? nullptr
                                : new sampler::Golden_ratio_pool(Num_dedicated_samplers)) {
    if (sampler_pool_) {
        for (uint32_t i = 0; i < Num_dedicated_samplers; ++i) {
            material_samplers_[i] = sampler_pool_->get(i, rng);
        }
    } else {
        for (auto& s : material_samplers_) {
            s = &sampler_;
        }
    }
}

Pathtracer::~Pathtracer() {
    delete sampler_pool_;
}

void Pathtracer::prepare(Scene const& /*scene*/, uint32_t num_samples_per_pixel) {
    sampler_.resize(num_samples_per_pixel, settings_.num_samples, 1, 1);

    for (auto s : material_samplers_) {
        s->resize(num_samples_per_pixel, settings_.num_samples, 1, 1);
    }
}

void Pathtracer::start_pixel() {
    sampler_.start_pixel();

    for (auto& s : material_samplers_) {
        s->start_pixel();
    }
}

float4 Pathtracer::li(Ray& ray, Intersection& intersection, Worker& worker,
                      Interface_stack const& initial_stack) {
    float const num_samples_reciprocal = 1.f / float(settings_.num_samples);

    float4 result = float4(0.f);

    for (uint32_t i = settings_.num_samples; i > 0; --i) {
        worker.reset_interface_stack(initial_stack);

        Ray split_ray = ray;

        Intersection split_intersection = intersection;

        result += num_samples_reciprocal * integrate(split_ray, split_intersection, worker);
    }

    return result;
}

float4 Pathtracer::integrate(Ray& ray, Intersection& intersection, Worker& worker) {
    Filter filter = Filter::Undefined;

    Bxdf_sample sample_result;

    bool primary_ray     = true;
    bool transparent     = true;
    bool from_subsurface = false;

    float3 throughput(1.f);
    float3 result(0.f);

#ifdef ONLY_CAUSTICS
    bool caustic_path = false;
#endif

    for (;;) {
        float3 const wo = -ray.direction;

        bool const avoid_caustics = settings_.avoid_caustics & (!primary_ray);

        auto const& material_sample = worker.sample_material(
            ray, wo, intersection, filter, avoid_caustics, from_subsurface, sampler_);

        if (material_sample.same_hemisphere(wo)) {
#ifdef ONLY_CAUSTICS
            if (caustic_path) {
                result += throughput * material_sample.radiance();
            }
#else
            result += throughput * material_sample.radiance();
#endif
        }

        if (material_sample.is_pure_emissive()) {
            transparent &= (!intersection.visible_in_camera(worker)) & (ray.max_t() >= Ray_max_t);
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

        material_sample.sample(material_sampler(ray.depth), sample_result);
        if (0.f == sample_result.pdf) {
            break;
        }

        if (sample_result.type.is(Bxdf_type::Caustic)) {
#ifdef ONLY_CAUSTICS
            caustic_path |= maybe_caustic;
#else
            if (avoid_caustics) {
                break;
            }
#endif
        } else if (sample_result.type.no(Bxdf_type::Straight)) {
            primary_ray = false;
            filter      = Filter::Nearest;
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

            transparent = false;

            from_subsurface = false;
        }

        if (material_sample.ior_greater_one()) {
            ++ray.depth;
        }

        ray.max_t() = Ray_max_t;

        if (sample_result.type.is(Bxdf_type::Transmission)) {
            worker.interface_change(sample_result.wi, intersection);
        }

        from_subsurface |= intersection.subsurface;

        if (!worker.interface_stack().empty()) {
            float3     vli;
            float3     vtr;
            auto const hit = worker.volume(ray, intersection, filter, vli, vtr);

            result += throughput * vli;

            throughput *= vtr;

            if ((Event::Abort == hit) | (Event::Absorb == hit)) {
                break;
            }
        } else if (!worker.intersect_and_resolve_mask(ray, intersection, filter)) {
            break;
        }
    }

    return compose_alpha(result, throughput, transparent);
}

sampler::Sampler& Pathtracer::material_sampler(uint32_t bounce) {
    if (Num_dedicated_samplers > bounce) {
        return *material_samplers_[bounce];
    }

    return sampler_;
}

Pathtracer_pool::Pathtracer_pool(uint32_t num_integrators, bool progressive, uint32_t num_samples,
                                 uint32_t min_bounces, uint32_t max_bounces, bool enable_caustics)
    : Typed_pool<Pathtracer>(num_integrators),
      settings_{num_samples, min_bounces, max_bounces, !enable_caustics},
      progressive_(progressive) {}

Integrator* Pathtracer_pool::get(uint32_t id, rnd::Generator& rng) const {
    if (uint32_t const zero = 0;
        0 == std::memcmp(&zero, static_cast<void*>(&integrators_[id]), 4)) {
        return new (&integrators_[id]) Pathtracer(rng, settings_, progressive_);
    }

    return &integrators_[id];
}

}  // namespace rendering::integrator::surface
