#include "pm.hpp"
#include "base/math/vector3.inl"
#include "base/memory/align.hpp"
#include "base/random/generator.inl"
#include "base/spectrum/rgb.hpp"
#include "rendering/integrator/integrator_helper.hpp"
#include "rendering/integrator/surface/surface_integrator.inl"
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

PM::PM(Settings const& settings, bool progressive)
    : settings_(settings),
      sampler_pool_(progressive ? nullptr
                                : new sampler::Golden_ratio_pool(Num_dedicated_samplers)) {
    if (sampler_pool_) {
        for (uint32_t i = 0; i < Num_dedicated_samplers; ++i) {
            material_samplers_[i] = sampler_pool_->get(i, 1, 1);
        }
    } else {
        for (auto& s : material_samplers_) {
            s = &sampler_;
        }
    }
}

PM::~PM() {
    delete sampler_pool_;
}

void PM::prepare(uint32_t num_samples_per_pixel) {
    sampler_.resize(num_samples_per_pixel);

    for (auto s : material_samplers_) {
        s->resize(num_samples_per_pixel);
    }
}

void PM::start_pixel(RNG& rng) {
    sampler_.start_pixel(rng);

    for (auto s : material_samplers_) {
        s->start_pixel(rng);
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

        if (sample_result.type.is(Bxdf_type::Caustic)) {
            treat_as_singular = sample_result.type.is(Bxdf_type::Specular);
        } else if (sample_result.type.no(Bxdf_type::Straight)) {
            filter            = Filter::Nearest;
            primary_ray       = false;
            treat_as_singular = false;
        }

        if (sample_result.type.no(Bxdf_type::Specular)) {
            if (ray.depth > 0 || settings_.photons_not_only_through_specular) {
                result += throughput * worker.photon_li(isec, mat_sample);
            }

            if ((sample_result.type.no(Bxdf_type::Transmission) &&
                 mat_sample.same_hemisphere(wo)) ||
                isec.subsurface) {
                break;
            }
        }

        if (ray.depth >= Max_bounces - 1) {
            break;
        }

        if (0.f == ray.wavelength) {
            ray.wavelength = sample_result.wavelength;
        }

        if (sample_result.type.is(Bxdf_type::Straight)) {
            ray.min_t() = scene::offset_f(ray.max_t());

            if (sample_result.type.no(Bxdf_type::Transmission)) {
                ++ray.depth;
            }
        } else {
            throughput *= sample_result.reflection / sample_result.pdf;

            ray.origin = mat_sample.offset_p(isec.geo.p, sample_result.wi, isec.subsurface);
            ray.set_direction(sample_result.wi);
            ++ray.depth;
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
        return *material_samplers_[bounce];
    }

    return sampler_;
}

PM_pool::PM_pool(uint32_t num_integrators, bool progressive, uint32_t min_bounces,
                 uint32_t max_bounces, bool photons_only_through_specular)
    : Typed_pool<PM>(num_integrators),
      settings_{min_bounces, max_bounces, !photons_only_through_specular},
      progressive_(progressive) {}

Integrator* PM_pool::get(uint32_t id) const {
    if (uint32_t const zero = 0;
        0 == std::memcmp(&zero, static_cast<void*>(&integrators_[id]), 4)) {
        return new (&integrators_[id]) PM(settings_, progressive_);
    }

    return &integrators_[id];
}

}  // namespace rendering::integrator::surface
