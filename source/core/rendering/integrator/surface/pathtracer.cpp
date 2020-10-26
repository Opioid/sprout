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

#include "base/debug/assert.hpp"

//#define ONLY_CAUSTICS

using namespace scene;

namespace rendering::integrator::surface {

Pathtracer::Pathtracer(Settings const& settings, bool progressive)
    : settings_(settings),
      sampler_pool_(progressive ? nullptr
                                : new sampler::Golden_ratio_pool(Num_dedicated_samplers)) {
    if (sampler_pool_) {
        for (uint32_t i = 0; i < Num_dedicated_samplers; ++i) {
            material_samplers_[i] = sampler_pool_->get(i);
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

void Pathtracer::start_pixel(RNG& rng) {
    sampler_.start_pixel(rng);

    for (auto& s : material_samplers_) {
        s->start_pixel(rng);
    }
}

float4 Pathtracer::li(Ray& ray, Intersection& isec, Worker& worker,
                      Interface_stack const& initial_stack, AOV& aov) {
    float const num_samples_reciprocal = 1.f / float(settings_.num_samples);

    float4 result = float4(0.f);

    for (uint32_t i = settings_.num_samples; i > 0; --i) {
        worker.reset_interface_stack(initial_stack);

        Ray split_ray = ray;

        Intersection split_intersection = isec;

        result += num_samples_reciprocal * integrate(split_ray, split_intersection, worker);
    }

    return result;
}

float4 Pathtracer::integrate(Ray& ray, Intersection& isec, Worker& worker) {
    Filter filter = Filter::Undefined;

    Bxdf_sample sample_result;

    bool primary_ray     = true;
    bool transparent     = true;
    bool from_subsurface = false;

    float3 throughput(1.f);
    float3 result(0.f);
    float3 wo1(0.f);

    float alpha = 0.f;

#ifdef ONLY_CAUSTICS
    bool caustic_path = false;
#endif

    for (;;) {
        float3 const wo = -ray.direction;

        bool const avoid_caustics = settings_.avoid_caustics & (!primary_ray);

        auto const& mat_sample = worker.sample_material(ray, wo, wo1, isec, filter, alpha,
                                                        avoid_caustics, from_subsurface, sampler_);

        alpha = mat_sample.alpha();

        wo1 = wo;

        if (mat_sample.same_hemisphere(wo)) {
#ifdef ONLY_CAUSTICS
            if (caustic_path) {
                result += throughput * mat_sample.radiance();
            }
#else
            result += throughput * mat_sample.radiance();
#endif
        }

        if (mat_sample.is_pure_emissive()) {
            transparent &= (!isec.visible_in_camera(worker)) & (ray.max_t() >= Ray_max_t);
            break;
        }

        if (ray.depth >= settings_.max_bounces) {
            break;
        }

        if (ray.depth > settings_.min_bounces) {
            if (russian_roulette(throughput, sampler_.generate_sample_1D(worker.rng()))) {
                break;
            }
        }

        mat_sample.sample(material_sampler(ray.depth), worker.rng(), sample_result);
        if (0.f == sample_result.pdf) {
            break;
        }

        if (sample_result.type.is(Bxdf_type::Caustic)) {
#ifdef ONLY_CAUSTICS
            caustic_path |= !primary_ray;
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

            result += throughput * vli;

            throughput *= vtr;

            if ((Event::Abort == hit) | (Event::Absorb == hit)) {
                SOFT_ASSERT(all_finite_and_positive(result));
                break;
            }
        } else if (!worker.intersect_and_resolve_mask(ray, isec, filter)) {
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

Integrator* Pathtracer_pool::get(uint32_t id) const {
    if (uint32_t const zero = 0;
        0 == std::memcmp(&zero, static_cast<void*>(&integrators_[id]), 4)) {
        return new (&integrators_[id]) Pathtracer(settings_, progressive_);
    }

    return &integrators_[id];
}

}  // namespace rendering::integrator::surface
