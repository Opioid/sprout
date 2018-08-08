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

void Pathtracer::resume_pixel(uint32_t sample, rnd::Generator& scramble) noexcept {
    sampler_.resume_pixel(sample, scramble);

    for (auto& s : material_samplers_) {
        s.resume_pixel(sample, scramble);
    }
}

float3 Pathtracer::li(Ray& ray, Intersection& intersection, Worker& worker) noexcept {
    float const num_samples_reciprocal = 1.f / static_cast<float>(settings_.num_samples);

    float3 result = float3::identity();

    for (uint32_t i = settings_.num_samples; i > 0; --i) {
        Ray split_ray = ray;

        Intersection split_intersection = intersection;

        result += num_samples_reciprocal * integrate(split_ray, split_intersection, worker);
    }

    return result;
}

float3 Pathtracer::integrate(Ray& ray, Intersection& intersection, Worker& worker) noexcept {
    Sampler_filter filter = Sampler_filter::Undefined;

    Bxdf_sample sample_result;

    bool primary_ray = true;

    float3 throughput(1.f);
    float3 result(0.f);

    for (;;) {
        float3 const wo = -ray.direction;

        bool const avoid_caustics = settings_.avoid_caustics && !primary_ray &&
                                    worker.interface_stack().top_is_vacuum();

        auto const& material_sample = intersection.sample(wo, ray, filter, avoid_caustics, sampler_,
                                                          worker);

        if (material_sample.same_hemisphere(wo)) {
            result += throughput * material_sample.radiance();
        }

        if (material_sample.is_pure_emissive()) {
            break;
        }

        if (ray.depth >= settings_.max_bounces) {
            break;
        }

        if (ray.depth > settings_.min_bounces) {
            float const q = settings_.path_continuation_probability;
            if (rendering::russian_roulette(throughput, q, sampler_.generate_sample_1D())) {
                break;
            }
        }

        material_sample.sample(material_sampler(ray.depth), sample_result);
        if (0.f == sample_result.pdf) {
            break;
        }

        bool const singular = sample_result.type.test_any(Bxdf_type::Specular,
                                                          Bxdf_type::Transmission);

        if (singular) {
            if (avoid_caustics && material_sample.ior_greater_one()) {
                break;
            }
        } else {
            primary_ray = false;
            filter      = Sampler_filter::Nearest;
        }

        float const ray_offset = take_settings_.ray_offset_factor * intersection.geo.epsilon;

        if (material_sample.ior_greater_one()) {
            throughput *= sample_result.reflection / sample_result.pdf;

            ray.origin = intersection.geo.p;
            ray.set_direction(sample_result.wi);
            ray.min_t = ray_offset;
            ++ray.depth;
        } else {
            ray.min_t = ray.max_t + ray_offset;
        }

        ray.max_t = scene::Ray_max_t;

        if (sample_result.type.test(Bxdf_type::Transmission)) {
            worker.interface_change(sample_result.wi, intersection);
        }

        if (!worker.interface_stack().empty()) {
            float3     vli, vtr;
            bool const hit = worker.volume(ray, intersection, filter, vli, vtr);

            result += throughput * vli;
            throughput *= vtr;

            if (!hit) {
                break;
            }
        } else if (!worker.intersect_and_resolve_mask(ray, intersection, filter)) {
            break;
        }
    }

    return result;
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
                                       float path_termination_probability,
                                       bool  enable_caustics) noexcept
    : Factory(take_settings),
      integrators_(memory::allocate_aligned<Pathtracer>(num_integrators)),
      settings_{num_samples, min_bounces, max_bounces, 1.f - path_termination_probability,
                !enable_caustics} {}

Pathtracer_factory::~Pathtracer_factory() noexcept {
    memory::free_aligned(integrators_);
}

Integrator* Pathtracer_factory::create(uint32_t id, rnd::Generator& rng) const noexcept {
    return new (&integrators_[id]) Pathtracer(rng, take_settings_, settings_);
}

}  // namespace rendering::integrator::surface
