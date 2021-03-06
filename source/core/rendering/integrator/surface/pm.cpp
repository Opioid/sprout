#include "pm.hpp"
#include "base/math/vector3.inl"
#include "base/memory/align.hpp"
#include "base/random/generator.inl"
#include "base/spectrum/rgb.hpp"
#include "rendering/integrator/integrator.inl"
#include "rendering/integrator/integrator_helper.hpp"
#include "rendering/rendering_worker.inl"
#include "rendering/sensor/aov/value.inl"
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

#define ONLY_CAUSTICS

namespace rendering::integrator::surface {

PM::PM(Settings const& settings, uint32_t max_samples_per_pixel, bool progressive)
    : settings_(settings) {
    if (progressive) {
        sampler_pool_ = new sampler::Random_pool(Num_dedicated_samplers);
    } else {
        sampler_pool_ = new sampler::Golden_ratio_pool(Num_dedicated_samplers);
    }

    for (uint32_t i = 0; i < Num_dedicated_samplers; ++i) {
        sampler_pool_->create(i, 1, 1, max_samples_per_pixel);
    }
}

PM::~PM() {
    delete sampler_pool_;
}

void PM::start_pixel(RNG& rng) {
    sampler_.start_pixel(rng);

    for (uint32_t i = 0; i < Num_dedicated_samplers; ++i) {
        sampler_pool_->get(i).start_pixel(rng);
    }
}

float4 PM::li(Ray& ray, Intersection& isec, Worker& worker, Interface_stack const& initial_stack,
              AOV* aov) {
    static uint32_t constexpr Max_bounces = 16;

    worker.reset_interface_stack(initial_stack);

    Filter filter = Filter::Undefined;

    Bxdf_sample sample_result;

    float3 throughput(1.f);

    float3 result = float3(0.f);

    bool const avoid_caustics = true;

    bool primary_ray       = true;
    bool treat_as_singular = true;
    bool direct            = true;

    for (uint32_t i = ray.depth;; ++i) {
        float3 const wo = -ray.direction;

        auto const& mat_sample = isec.sample(wo, ray, filter, 0.f, avoid_caustics, sampler_,
                                             worker);

#ifndef ONLY_CAUSTICS
        if (treat_as_singular & mat_sample.same_hemisphere(wo)) {
            result += throughput * mat_sample.radiance();
        }
#endif

        if (aov) {
            common_AOVs(throughput, ray, isec, mat_sample, primary_ray, worker, *aov);
        }

        if (mat_sample.is_pure_emissive()) {
            break;
        }

        mat_sample.sample(material_sampler(ray.depth), worker.rng(), sample_result);
        if (0.f == sample_result.pdf) {
            break;
        }

        if (sample_result.type.is(Bxdf_type::Specular)) {
            treat_as_singular = true;
        } else if (sample_result.type.no(Bxdf_type::Straight)) {
            treat_as_singular = false;

            if (primary_ray) {
                primary_ray = false;
                filter      = Filter::Nearest;

                if (bool const indirect = !direct & (0 != ray.depth);
                    indirect || settings_.photons_not_only_through_specular) {
                    result += throughput * worker.photon_li(isec, mat_sample);
                }
            }
        }

        if (ray.depth >= Max_bounces - 1) {
            break;
        }

        if (0.f == ray.wavelength) {
            ray.wavelength = sample_result.wavelength;
        }

        throughput *= sample_result.reflection / sample_result.pdf;

        if (sample_result.type.is(Bxdf_type::Straight)) {
            ray.min_t() = scene::offset_f(ray.max_t());

            if (sample_result.type.no(Bxdf_type::Transmission)) {
                ++ray.depth;
            }
        } else {
            ray.origin = isec.offset_p(sample_result.wi);
            ray.set_direction(sample_result.wi);
            ++ray.depth;

            direct = false;
        }

        ray.max_t() = scene::Ray_max_t;

        if (sample_result.type.is(Bxdf_type::Transmission)) {
            worker.interface_change(sample_result.wi, isec);
        }

        if (!worker.interface_stack().empty()) {
            float3     vli;
            float3     vtr;
            auto const hit = worker.volume(ray, isec, filter, vli, vtr);

            // result += throughput * vli;
            throughput *= vtr;

            if (Event::Abort == hit) {
                break;
            }
        } else if (!worker.intersect_and_resolve_mask(ray, isec, filter)) {
            break;
        }
    }

    return float4(result, 1.f);
}

sampler::Sampler& PM::material_sampler(uint32_t bounce) {
    if (Num_dedicated_samplers > bounce) {
        return sampler_pool_->get(bounce);
    }

    return sampler_;
}

PM_pool::PM_pool(uint32_t num_integrators, bool progressive, uint32_t min_bounces,
                 uint32_t max_bounces, bool photons_only_through_specular)
    : Typed_pool<PM, Integrator>(num_integrators),
      settings_{min_bounces, max_bounces, !photons_only_through_specular},
      progressive_(progressive) {}

Integrator* PM_pool::create(uint32_t id, uint32_t max_samples_per_pixel) const {
    return new (&integrators_[id]) PM(settings_, max_samples_per_pixel, progressive_);
}

}  // namespace rendering::integrator::surface
