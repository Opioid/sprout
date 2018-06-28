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

    float3 throughput(1.f);

    float3 result = float3::identity();

    bool const avoid_caustics = true;

    for (uint32_t i = 4; i > 0; --i) {
        float3 const wo = -ray.direction;

        auto const& material_sample = intersection.sample(wo, ray, filter, avoid_caustics, sampler_,
                                                          worker);

        if (material_sample.same_hemisphere(wo)) {
            result += material_sample.radiance();
        }

        if (material_sample.is_pure_emissive()) {
            return result;
        }

        material_sample.sample(sampler_, sample_result);
        if (0.f == sample_result.pdf) {
            break;
        }

        bool const singular = sample_result.type.test_any(Bxdf_type::Specular,
                                                          Bxdf_type::Transmission);

        if (!singular) {
            result += throughput * worker.photon_li(intersection.geo.p, material_sample);
            break;
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

            // result += throughput * vli;
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
    float3 const axis = to - from;

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

        auto const bxdf = sample.evaluate(wi);

        return thing * (tv * tr) * bxdf.reflection;
    }

    return float3(0.f);
}

float3 Lighttracer::connect(f_float3 from, f_float3 to, Material_sample const& from_sample,
                            Material_sample const& to_sample, Ray const& history, float ray_offset,
                            Worker& worker) {
    float3 const axis = to - from;

    float3 const wi = math::normalize(axis);

    Ray shadow_ray(to, wi, ray_offset, math::length(axis) - ray_offset, history.depth, history.time,
                   history.wavelength);

    float3 const tv = worker.tinted_visibility(shadow_ray, Sampler_filter::Nearest);
    if (math::any_greater_zero(tv)) {
        float3 const tr = worker.transmittance(shadow_ray);

        auto const from_bxdf = from_sample.evaluate(wi);
        auto const to_bxdf   = to_sample.evaluate(-wi);

        return from_bxdf.reflection * to_bxdf.reflection;
    }

    return float3(0.f);
}

float3 Lighttracer::direct_light(Ray const& ray, Intersection const& intersection,
                                 const Material_sample& material_sample, Sampler_filter filter,
                                 Worker& worker) {
    float3 result(0.f);

    if (!material_sample.ior_greater_one()) {
        return result;
    }

    Ray shadow_ray;
    shadow_ray.origin = intersection.geo.p;
    shadow_ray.min_t  = take_settings_.ray_offset_factor * intersection.geo.epsilon;
    shadow_ray.depth  = ray.depth + 1;
    shadow_ray.time   = ray.time;

    for (uint32_t i = 1; i > 0; --i) {
        auto const light = worker.scene().random_light(rng_.random_float());

        scene::light::Sample_to light_sample;
        if (light.ref.sample(intersection.geo.p, material_sample.geometric_normal(), ray.time,
                             material_sample.is_translucent(), sampler_, 0, Sampler_filter::Nearest,
                             worker, light_sample)) {
            shadow_ray.set_direction(light_sample.shape.wi);
            float const offset = take_settings_.ray_offset_factor * light_sample.shape.epsilon;
            shadow_ray.max_t   = light_sample.shape.t - offset;

            //	float3 const tv = worker.tinted_visibility(shadow_ray, filter);
            float3 const tv = worker.tinted_visibility(shadow_ray, intersection, filter);
            if (math::any_greater_zero(tv)) {
                float3 const tr = worker.transmittance(shadow_ray);

                auto const bxdf = material_sample.evaluate(light_sample.shape.wi);

                result += (tv * tr) * (light_sample.radiance * bxdf.reflection) /
                          (light.pdf * light_sample.shape.pdf);
            }
        }
    }

    return result;
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
