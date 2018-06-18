#include "lighttracer.hpp"
#include "base/math/vector3.inl"
#include "base/memory/align.hpp"
#include "base/random/generator.inl"
#include "base/spectrum/rgb.hpp"
#include "rendering/integrator/integrator_helper.hpp"
#include "rendering/rendering_worker.hpp"
#include "scene/light/light.hpp"
#include "scene/light/light_sample.hpp"
#include "scene/material/bxdf.hpp"
#include "scene/material/material.hpp"
#include "scene/material/material_sample.inl"
#include "scene/prop/interface_stack.inl"
#include "scene/prop/prop_intersection.inl"
#include "scene/scene.hpp"
#include "scene/scene_constants.hpp"
#include "scene/scene_ray.inl"

namespace rendering::integrator::surface {

Lighttracer::Lighttracer(rnd::Generator& rng, take::Settings const& take_settings,
                         Settings const& settings)
    : Integrator(rng, take_settings),
      settings_(settings),
      sampler_(rng),
      material_samplers_{rng, rng, rng} {}

Lighttracer::~Lighttracer() {}

void Lighttracer::prepare(Scene const& /*scene*/, uint32_t num_samples_per_pixel) {
    sampler_.resize(num_samples_per_pixel, 1, 1, 1);

    for (auto& s : material_samplers_) {
        s.resize(num_samples_per_pixel, 1, 1, 1);
    }
}

void Lighttracer::resume_pixel(uint32_t sample, rnd::Generator& scramble) {
    sampler_.resume_pixel(sample, scramble);

    for (auto& s : material_samplers_) {
        s.resume_pixel(sample, scramble);
    }
}

float3 Lighttracer::li(Ray& ray, Intersection& intersection, Worker& worker) {
    Sampler_filter filter = Sampler_filter::Undefined;

    Bxdf_sample sample_result;

    bool primary_ray = true;

    float3 throughput(1.f);
    float3 result(0.f);

    float3 const wo = -ray.direction;

    auto const& first_sample = intersection.sample(wo, ray, filter, false, sampler_, worker, 1);

    if (first_sample.same_hemisphere(wo)) {
        result += first_sample.radiance();
    }

    if (first_sample.is_pure_emissive()) {
        return result;
    }

    Ray    light_ray;
    float3 radiance;

    if (!generate_light_ray(ray.time, worker, light_ray, radiance)) {
        return result;
    }

    float const ray_offset = take_settings_.ray_offset_factor * intersection.geo.epsilon;

    result = radiance * connect(light_ray.origin, intersection.geo.p, first_sample, light_ray,
                                ray_offset, worker);

    /*
    if (!worker.intersect_and_resolve_mask(light_ray, intersection, filter)) {
        return result;
    }

    {

        float3 const wi = -light_ray.direction;
        //   float3 const wo = -ray.direction;

        auto const& material_sample = intersection.sample(wi, ray, filter, false, sampler_, worker);

        float const ray_offset = take_settings_.ray_offset_factor * intersection.geo.epsilon;

        float3 const eye_axis = ray.origin - intersection.geo.p;

        float3 const wo = math::normalize(eye_axis);

        Ray shadow_ray(intersection.geo.p, math::normalize(eye_axis), ray_offset,
                       math::length(eye_axis), ray.depth, ray.time, ray.wavelength);

        float3 const tv = worker.tinted_visibility(shadow_ray, intersection, filter);
        if (math::any_greater_zero(tv)) {
            //    float3 const tr = worker.transmittance(shadow_ray);

            auto const bxdf = material_sample.evaluate(wo);

            float const n_dot_wi = std::abs(math::dot(wi, material_sample.geometric_normal()));

            float const n_dot_wo = std::abs(math::dot(wo, material_sample.geometric_normal()));

            float const ln_dot_wi = math::dot(float3(0.f, 1.f, 0.f), wi);

            float const wi_dot_wo = math::saturate(math::dot(wi, wo));

            float const eye_dot_wo = math::saturate(math::dot(-ray.direction, wo));
            //   result += float3(n_dot_wo);

            return float3(1.f, 0.f, 0.f);
        } else {
            return float3(0.f, 0.f, 0.f);
        }
    }
    */

    return result;
}

bool Lighttracer::generate_light_ray(float time, Worker& worker, Ray& ray, float3& radiance) {
    float const select = sampler_.generate_sample_1D(1);

    auto const light = worker.scene().random_light(select);

    scene::light::Sample_from light_sample;
    if (!light.ref.sample(time, sampler_, 0, Sampler_filter::Nearest, worker, light_sample)) {
        return false;
    }

    ray.origin = light_sample.shape.p;
    ray.set_direction(light_sample.shape.dir);
    ray.min_t = take_settings_.ray_offset_factor * light_sample.shape.epsilon;
    ray.max_t = scene::Ray_max_t;

    radiance = light_sample.radiance / light_sample.shape.pdf;

    return true;
}

float3 Lighttracer::connect(f_float3 from, f_float3 to, Material_sample const& sample,
                            Ray const& history, float ray_offset, Worker& worker) {
    float3 const axis = from - to;

    float3 const wi = math::normalize(axis);

    Ray shadow_ray(to, wi, ray_offset, math::length(axis) - ray_offset, history.depth, history.time,
                   history.wavelength);

    float3 const tv = worker.tinted_visibility(shadow_ray, Sampler_filter::Nearest);
    if (math::any_greater_zero(tv)) {
        float3 const tr = worker.transmittance(shadow_ray);

        float const thing = math::dot(wi, float3(0.f, 1.f, 0.f));

        if (thing <= 0.f) {
            return float3(0.f);
        }

        // ----

        float sl = math::squared_length(axis);

        float const thong = (sl / (thing * 0.0484f));

        auto const bxdf = sample.evaluate(wi);

        return (thing / thong) * (tv * tr) * bxdf.reflection;  // / (1.f / bxdf.pdf);
    }

    return float3(0.f);
}

sampler::Sampler& Lighttracer::material_sampler(uint32_t bounce) {
    if (Num_material_samplers > bounce) {
        return material_samplers_[bounce];
    }

    return sampler_;
}

size_t Lighttracer::num_bytes() const {
    size_t sampler_bytes = 0;

    for (auto& s : material_samplers_) {
        sampler_bytes += s.num_bytes();
    }

    return sizeof(*this) + sampler_.num_bytes() + sampler_bytes;
}

Lighttracer_factory::Lighttracer_factory(take::Settings const& take_settings,
                                         uint32_t num_integrators, uint32_t min_bounces,
                                         uint32_t max_bounces, float path_termination_probability)
    : Factory(take_settings),
      integrators_(memory::allocate_aligned<Lighttracer>(num_integrators)),
      settings_{min_bounces, max_bounces, 1.f - path_termination_probability} {}

Lighttracer_factory::~Lighttracer_factory() {
    memory::free_aligned(integrators_);
}

Integrator* Lighttracer_factory::create(uint32_t id, rnd::Generator& rng) const {
    return new (&integrators_[id]) Lighttracer(rng, take_settings_, settings_);
}

uint32_t Lighttracer_factory::max_sample_depth() const {
    return 2;
}

}  // namespace rendering::integrator::surface
