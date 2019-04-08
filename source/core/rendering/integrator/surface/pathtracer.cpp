#include "pathtracer.hpp"
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

Pathtracer::Pathtracer(rnd::Generator& rng, take::Settings const& take_settings,
                       Settings const& settings) noexcept
    : Integrator(rng, take_settings),
      settings_(settings),
      sampler_(rng),
      material_samplers_{rng, rng, rng} {}

Pathtracer::~Pathtracer() noexcept {}

void Pathtracer::prepare(Scene const& /*scene*/, uint32_t num_samples_per_pixel) noexcept {
    sampler_.resize(num_samples_per_pixel, settings_.num_samples, 1, 1);

    for (auto& s : material_samplers_) {
        s.resize(num_samples_per_pixel, settings_.num_samples, 1, 1);
    }
}

void Pathtracer::start_pixel() noexcept {
    sampler_.start_pixel();

    for (auto& s : material_samplers_) {
        s.start_pixel();
    }
}

float4 Pathtracer::li(Ray& ray, Intersection& intersection, Worker& worker,
                      Interface_stack const& initial_stack) noexcept {
    float const num_samples_reciprocal = 1.f / static_cast<float>(settings_.num_samples);

    float4 result = float4(0.f);

    for (uint32_t i = settings_.num_samples; i > 0; --i) {
        worker.reset_interface_stack(initial_stack);

        Ray split_ray = ray;

        Intersection split_intersection = intersection;

        result += num_samples_reciprocal * integrate(split_ray, split_intersection, worker);
    }

    return result;
}

float4 Pathtracer::integrate(Ray& ray, Intersection& intersection, Worker& worker) noexcept {
    Filter filter = Filter::Undefined;

    Bxdf_sample sample_result;

    bool primary_ray = true;
    bool transparent = true;

    float3 throughput(1.f);
    float3 result(0.f);

    for (;;) {
        float3 const wo = -ray.direction;

        bool const avoid_caustics = settings_.avoid_caustics && !primary_ray &&
                                    worker.interface_stack().top_is_vacuum_or_not_scattering();

        auto const& material_sample = intersection.sample(wo, ray, filter, avoid_caustics, sampler_,
                                                          worker);

        if (material_sample.same_hemisphere(wo)) {
            result += throughput * material_sample.radiance();
        }

        if (material_sample.is_pure_emissive()) {
            transparent &= !intersection.prop->visible_in_camera() && ray.max_t >= scene::Ray_max_t;
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

        if (sample_result.type.test(Bxdf_type::Caustic)) {
            if (avoid_caustics) {
                break;
            }
        } else if (sample_result.type.test_not(Bxdf_type::Pass_through)) {
            primary_ray = false;
            filter      = Filter::Nearest;
        }

        if (0.f == ray.wavelength) {
            ray.wavelength = sample_result.wavelength;
        }

        if (material_sample.ior_greater_one()) {
            transparent &= sample_result.type.test(Bxdf_type::Pass_through);

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

            result += throughput * vli;
            throughput *= vtr;

            if (Event::Abort == hit || Event::Absorb == hit) {
                break;
            }
        } else if (!worker.intersect_and_resolve_mask(ray, intersection, filter)) {
            break;
        }
    }

    return compose_alpha(result, throughput, transparent);
}

sampler::Sampler& Pathtracer::material_sampler(uint32_t bounce) noexcept {
    if (Num_material_samplers > bounce) {
        return material_samplers_[bounce];
    }

    return sampler_;
}

size_t Pathtracer::num_bytes() const noexcept {
    size_t sampler_bytes = 0;

    for (auto& s : material_samplers_) {
        sampler_bytes += s.num_bytes();
    }

    return sizeof(*this) + sampler_.num_bytes() + sampler_bytes;
}

Pathtracer_factory::Pathtracer_factory(take::Settings const& take_settings,
                                       uint32_t num_integrators, uint32_t num_samples,
                                       uint32_t min_bounces, uint32_t max_bounces,
                                       bool enable_caustics) noexcept
    : Factory(take_settings),
      integrators_(memory::allocate_aligned<Pathtracer>(num_integrators)),
      settings_{num_samples, min_bounces, max_bounces, !enable_caustics} {}

Pathtracer_factory::~Pathtracer_factory() noexcept {
    memory::free_aligned(integrators_);
}

Integrator* Pathtracer_factory::create(uint32_t id, rnd::Generator& rng) const noexcept {
    return new (&integrators_[id]) Pathtracer(rng, take_settings_, settings_);
}

}  // namespace rendering::integrator::surface
