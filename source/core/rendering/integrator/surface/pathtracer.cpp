#include "pathtracer.hpp"
#include "base/math/vector3.inl"
#include "base/memory/align.hpp"
#include "base/random/generator.inl"
#include "base/spectrum/rgb.hpp"
#include "rendering/integrator/integrator_helper.hpp"
#include "rendering/integrator/surface/surface_integrator.inl"
#include "rendering/rendering_worker.inl"
#include "rendering/sensor/aov/value.inl"
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

Pathtracer::Pathtracer(Settings const& settings, uint32_t max_samples_per_pixel, bool progressive)
    : settings_(settings) {
    if (progressive) {
        sampler_pool_ = new sampler::Random_pool(Num_dedicated_samplers);
    } else {
        sampler_pool_ = new sampler::Golden_ratio_pool(Num_dedicated_samplers);
    }

    uint32_t const max_samples = max_samples_per_pixel * settings_.num_samples;

    for (uint32_t i = 0; i < Num_dedicated_samplers; ++i) {
        sampler_pool_->create(i, 2, 1, max_samples);
    }
}

Pathtracer::~Pathtracer() {
    delete sampler_pool_;
}

void Pathtracer::start_pixel(RNG& rng) {
    sampler_.start_pixel(rng);

    for (uint32_t i = 0; i < Num_dedicated_samplers; ++i) {
        sampler_pool_->get(i).start_pixel(rng);
    }
}

float4 Pathtracer::li(Ray& ray, Intersection& isec, Worker& worker,
                      Interface_stack const& initial_stack, AOV* aov) {
    float const num_samples_reciprocal = 1.f / float(settings_.num_samples);

    float4 result = float4(0.f);

    for (uint32_t i = settings_.num_samples; i > 0; --i) {
        worker.reset_interface_stack(initial_stack);

        Ray split_ray = ray;

        Intersection split_intersection = isec;

        result += num_samples_reciprocal * integrate(split_ray, split_intersection, worker, aov);
    }

    return result;
}

float4 Pathtracer::integrate(Ray& ray, Intersection& isec, Worker& worker, AOV* aov) {
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

    for (uint32_t i = 0;; ++i) {
        float3 const wo = -ray.direction;

        bool const avoid_caustics = settings_.avoid_caustics & (!primary_ray);

        Filter const filter = ((ray.depth <= 1) | primary_ray) ? Filter::Undefined
                                                               : Filter::Nearest;

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

        if (aov) {
            common_AOVs(throughput, ray, isec, mat_sample, primary_ray, worker, *aov);
        }

        if (mat_sample.is_pure_emissive()) {
            transparent &= (!isec.visible_in_camera(worker)) & (ray.max_t() >= Ray_max_t);
            break;
        }

        if (ray.depth >= settings_.max_bounces) {
            break;
        }

        if (ray.depth > settings_.min_bounces) {
            if (russian_roulette(throughput, sampler_.sample_1D(worker.rng()))) {
                break;
            }
        }

        mat_sample.sample(material_sampler(ray.depth), worker.rng(), sample_result);
        if (0.f == sample_result.pdf) {
            break;
        }

        if (sample_result.type.is(Bxdf_type::Specular)) {
#ifdef ONLY_CAUSTICS
            caustic_path |= !primary_ray;
#else
            if (avoid_caustics) {
                break;
            }
#endif
        } else if (sample_result.type.no(Bxdf_type::Straight)) {
            primary_ray = false;
        }

        if (sample_result.type != Bxdf_type::Straight_transmission) {
            ++ray.depth;
        }

        if (sample_result.type.is(Bxdf_type::Straight)) {
            ray.min_t() = offset_f(ray.max_t());
        } else {
            ray.origin = isec.offset_p(sample_result.wi);
            ray.set_direction(sample_result.wi);

            transparent     = false;
            from_subsurface = false;
        }

        ray.max_t() = Ray_max_t;

        if (0.f == ray.wavelength) {
            ray.wavelength = sample_result.wavelength;
        }

        throughput *= sample_result.reflection / sample_result.pdf;

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
        return sampler_pool_->get(bounce);
    }

    return sampler_;
}

Pathtracer_pool::Pathtracer_pool(uint32_t num_integrators, bool progressive, uint32_t num_samples,
                                 uint32_t min_bounces, uint32_t max_bounces, bool enable_caustics)
    : Typed_pool<Pathtracer>(num_integrators),
      settings_{num_samples, min_bounces, max_bounces, !enable_caustics},
      progressive_(progressive) {}

Integrator* Pathtracer_pool::create(uint32_t id, uint32_t max_samples_per_pixel) const {
    return new (&integrators_[id]) Pathtracer(settings_, max_samples_per_pixel, progressive_);
}

}  // namespace rendering::integrator::surface
