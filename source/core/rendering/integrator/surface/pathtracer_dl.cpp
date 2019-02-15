#include "pathtracer_dl.hpp"
#include "base/math/vector3.inl"
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

namespace rendering::integrator::surface {

using namespace scene;

Pathtracer_DL::Pathtracer_DL(rnd::Generator& rng, take::Settings const& take_settings,
                             Settings const& settings) noexcept
    : Integrator(rng, take_settings),
      settings_(settings),
      sampler_(rng),
      material_samplers_{rng, rng, rng},
      light_samplers_{rng, rng, rng} {}

void Pathtracer_DL::prepare(Scene const& /*scene*/, uint32_t num_samples_per_pixel) noexcept {
    sampler_.resize(num_samples_per_pixel, 1, 1, 1);

    for (auto& s : material_samplers_) {
        s.resize(num_samples_per_pixel, 1, 1, 1);
    }

    for (auto& s : light_samplers_) {
        s.resize(num_samples_per_pixel, settings_.num_light_samples, 1, 2);
    }
}

void Pathtracer_DL::start_pixel() noexcept {
    sampler_.start_pixel();

    for (auto& s : material_samplers_) {
        s.start_pixel();
    }

    for (auto& s : light_samplers_) {
        s.start_pixel();
    }
}

float3 Pathtracer_DL::li(Ray& ray, Intersection& intersection, Worker& worker,
                         Interface_stack const& initial_stack) noexcept {
    worker.reset_interface_stack(initial_stack);

    Filter filter = Filter::Undefined;

    Bxdf_sample sample_result;

    bool primary_ray       = true;
    bool treat_as_singular = true;
    bool evaluate_back     = true;

    float3 throughput(1.f);
    float3 result(0.f);

    for (uint32_t i = ray.depth;; ++i) {
        float3 const wo = -ray.direction;

        bool const avoid_caustics = settings_.avoid_caustics && !primary_ray &&
                                    worker.interface_stack().top_is_vacuum_or_not_scattering();

        auto& material_sample = intersection.sample(wo, ray, filter, avoid_caustics, sampler_,
                                                    worker);

        bool const same_side = material_sample.same_hemisphere(wo);

        if (treat_as_singular && same_side) {
            result += throughput * material_sample.radiance();
        }

        if (material_sample.is_pure_emissive()) {
            break;
        }

        evaluate_back = material_sample.do_evaluate_back(evaluate_back, same_side);

        result += throughput *
                  direct_light(ray, intersection, material_sample, evaluate_back, filter, worker);

        SOFT_ASSERT(all_finite_and_positive(result));

        if (ray.depth >= settings_.max_bounces - 1) {
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

        if (sample_result.type.test(Bxdf_type::Caustic)) {
            if (material_sample.ior_greater_one()) {
                if (avoid_caustics) {
                    break;
                }

                treat_as_singular = sample_result.type.test(Bxdf_type::Specular);
            }
        } else {
            primary_ray       = false;
            filter            = Filter::Nearest;
            treat_as_singular = false;
        }

        if (0.f == ray.wavelength) {
            ray.wavelength = sample_result.wavelength;
        }

        if (material_sample.ior_greater_one()) {
            throughput *= sample_result.reflection / sample_result.pdf;

            ray.set_direction(sample_result.wi);
            ++ray.depth;
        }

        ray.origin = material_sample.offset_p(intersection.geo.p, sample_result.wi);
        ray.min_t  = 0.f;
        ray.max_t  = scene::Ray_max_t;

        if (sample_result.type.test(Bxdf_type::Transmission)) {
            worker.interface_change(sample_result.wi, intersection);
        }

        if (!worker.interface_stack().empty()) {
            float3     vli, vtr;
            auto const hit = worker.volume(ray, intersection, filter, vli, vtr);

            if (treat_as_singular) {
                result += throughput * vli;
            }

            throughput *= vtr;

            if (Event::Abort == hit || Event::Absorb == hit) {
                break;
            }
        } else if (!worker.intersect_and_resolve_mask(ray, intersection, filter)) {
            break;
        }
    }

    return result;
}

float3 Pathtracer_DL::direct_light(Ray const& ray, Intersection const& intersection,
                                   Material_sample const& material_sample, bool evaluate_back,
                                   Filter filter, Worker& worker) noexcept {
    float3 result(0.f);

    if (!material_sample.ior_greater_one()) {
        return result;
    }

    Ray shadow_ray;
    shadow_ray.origin     = material_sample.offset_p(intersection.geo.p);
    shadow_ray.min_t      = 0.f;
    shadow_ray.depth      = ray.depth;
    shadow_ray.time       = ray.time;
    shadow_ray.wavelength = ray.wavelength;

    for (uint32_t i = settings_.num_light_samples; i > 0; --i) {
        float const select = light_sampler(ray.depth).generate_sample_1D(1);

        auto const light = worker.scene().random_light(select);

        shape::Sample_to light_sample;
        if (!light.ref.sample(intersection.geo.p, material_sample.geometric_normal(), ray.time,
                              material_sample.is_translucent(), light_sampler(ray.depth), 0, worker,
                              light_sample)) {
            continue;
        }

        shadow_ray.set_direction(light_sample.wi);
        shadow_ray.max_t = light_sample.t;

        if (float3 tv; worker.transmitted_visibility(shadow_ray, intersection, filter, tv)) {
            auto const bxdf = material_sample.evaluate_f(light_sample.wi, evaluate_back);

            float3 const radiance = light.ref.evaluate(light_sample, Filter::Nearest, worker);

            float const weight = 1.f / (light.pdf * light_sample.pdf);

            result += weight * (tv * radiance * bxdf.reflection);
        }
    }

    return result / static_cast<float>(settings_.num_light_samples);
}

sampler::Sampler& Pathtracer_DL::material_sampler(uint32_t bounce) noexcept {
    if (Num_material_samplers > bounce) {
        return material_samplers_[bounce];
    }

    return sampler_;
}

sampler::Sampler& Pathtracer_DL::light_sampler(uint32_t bounce) noexcept {
    if (Num_light_samplers > bounce) {
        return light_samplers_[bounce];
    }

    return sampler_;
}

size_t Pathtracer_DL::num_bytes() const noexcept {
    return sizeof(*this) + sampler_.num_bytes();
}

Pathtracer_DL_factory::Pathtracer_DL_factory(take::Settings const& take_settings,
                                             uint32_t num_integrators, uint32_t min_bounces,
                                             uint32_t max_bounces, uint32_t num_light_samples,
                                             bool enable_caustics) noexcept
    : Factory(take_settings),
      integrators_(memory::allocate_aligned<Pathtracer_DL>(num_integrators)) {
    settings_.min_bounces       = min_bounces;
    settings_.max_bounces       = max_bounces;
    settings_.num_light_samples = num_light_samples;
    settings_.avoid_caustics    = !enable_caustics;
}

Pathtracer_DL_factory::~Pathtracer_DL_factory() noexcept {
    memory::free_aligned(integrators_);
}

Integrator* Pathtracer_DL_factory::create(uint32_t id, rnd::Generator& rng) const noexcept {
    return new (&integrators_[id]) Pathtracer_DL(rng, take_settings_, settings_);
}

}  // namespace rendering::integrator::surface
