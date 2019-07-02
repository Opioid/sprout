#include "pm.hpp"
#include "base/math/vector3.inl"
#include "base/memory/align.hpp"
#include "base/random/generator.inl"
#include "base/spectrum/rgb.hpp"
#include "rendering/integrator/integrator_helper.hpp"
#include "rendering/rendering_worker.hpp"
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

namespace rendering::integrator::surface {

PM::PM(rnd::Generator& rng, take::Settings const& take_settings, Settings const& settings) noexcept
    : Integrator(rng, take_settings),
      settings_(settings),
      sampler_(rng),
      material_samplers_{rng, rng, rng} {}

PM::~PM() noexcept {}

void PM::prepare(Scene const& /*scene*/, uint32_t num_samples_per_pixel) noexcept {
    sampler_.resize(num_samples_per_pixel, 1, 1, 1);

    for (auto& s : material_samplers_) {
        s.resize(num_samples_per_pixel, 1, 1, 1);
    }
}

void PM::start_pixel() noexcept {
    sampler_.start_pixel();

    for (auto& s : material_samplers_) {
        s.start_pixel();
    }
}

float4 PM::li(Ray& ray, Intersection& intersection, Worker& worker,
              Interface_stack const& initial_stack) noexcept {
    static uint32_t constexpr Max_bounces = 16;

    worker.reset_interface_stack(initial_stack);

    Filter filter = Filter::Undefined;

    Bxdf_sample sample_result;

    float3 throughput(1.f);

    float3 result = float3(0.f);

    bool const avoid_caustics = true;

    for (uint32_t i = ray.depth;; ++i) {
        float3 const wo = -ray.direction;

        auto const& material_sample = intersection.sample(wo, ray, filter, avoid_caustics, sampler_,
                                                          worker);

        if (material_sample.same_hemisphere(wo)) {
            result += material_sample.radiance();
        }

        if (material_sample.is_pure_emissive()) {
            return float4(result, 1.f);
        }

        material_sample.sample(material_sampler(ray.depth), sample_result);
        if (0.f == sample_result.pdf) {
            break;
        }

        if (!sample_result.type.test(Bxdf_type::Caustic)) {
            result += throughput * worker.photon_li(intersection, material_sample);
            break;
        }

        if (ray.depth >= Max_bounces - 1) {
            break;
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

            // result += throughput * vli;
            throughput *= vtr;

            if (Event::Abort == hit) {
                break;
            }
        } else if (!worker.intersect_and_resolve_mask(ray, intersection, filter)) {
            break;
        }
    }

    return float4(result, 1.f);
}

bool PM::generate_light_ray(uint64_t time, Worker& worker, Ray& ray, float3& radiance) noexcept {
    Scene const& scene = worker.scene();

    float const select = sampler_.generate_sample_1D(1);

    auto const light = scene.random_light(select);

    scene::shape::Sample_from light_sample;
    if (!light.ref.sample(time, sampler_, 0, scene.aabb(), worker, light_sample)) {
        return false;
    }

    ray.origin = scene::offset_ray(light_sample.p, light_sample.dir);
    ray.set_direction(light_sample.dir);
    ray.min_t      = 0.f;
    ray.max_t      = scene::Ray_max_t;
    ray.time       = time;
    ray.wavelength = 0.f;

    radiance = light.ref.evaluate(light_sample, Filter::Nearest, worker) /
               (light.pdf * light_sample.pdf);

    return true;
}

float3 PM::direct_light(Ray const& ray, Intersection const& intersection,
                        Material_sample const& material_sample, Filter filter,
                        Worker& worker) noexcept {
    float3 result(0.f);

    if (!material_sample.ior_greater_one()) {
        return result;
    }

    Ray shadow_ray;
    shadow_ray.origin = intersection.geo.p;
    shadow_ray.min_t  = 0.f;
    shadow_ray.depth  = ray.depth + 1;
    shadow_ray.time   = ray.time;

    for (uint32_t i = 1; i > 0; --i) {
        auto const light = worker.scene().random_light(rng_.random_float());

        scene::shape::Sample_to light_sample;
        if (light.ref.sample(intersection.geo.p, material_sample.geometric_normal(), ray.time,
                             material_sample.is_translucent(), sampler_, 0, worker, light_sample)) {
            shadow_ray.set_direction(light_sample.wi);
            shadow_ray.max_t = light_sample.t;

            float3 tv;
            if (worker.transmitted_visibility(shadow_ray, material_sample.wo(), intersection,
                                              filter, tv)) {
                auto const bxdf = material_sample.evaluate_f(light_sample.wi, true);

                float3 const radiance = light.ref.evaluate(light_sample, Filter::Nearest, worker);

                result += (tv * radiance * bxdf.reflection) / (light.pdf * light_sample.pdf);
            }
        }
    }

    return result;
}

sampler::Sampler& PM::material_sampler(uint32_t bounce) noexcept {
    if (Num_material_samplers > bounce) {
        return material_samplers_[bounce];
    }

    return sampler_;
}

size_t PM::num_bytes() const noexcept {
    size_t sampler_bytes = 0;

    for (auto& s : material_samplers_) {
        sampler_bytes += s.num_bytes();
    }

    return sizeof(*this) + sampler_.num_bytes() + sampler_bytes;
}

PM_factory::PM_factory(take::Settings const& take_settings, uint32_t num_integrators,
                       uint32_t min_bounces, uint32_t max_bounces) noexcept
    : Factory(take_settings),
      integrators_(memory::allocate_aligned<PM>(num_integrators)),
      settings_{min_bounces, max_bounces} {}

PM_factory::~PM_factory() noexcept {
    memory::free_aligned(integrators_);
}

Integrator* PM_factory::create(uint32_t id, rnd::Generator& rng) const noexcept {
    return new (&integrators_[id]) PM(rng, take_settings_, settings_);
}

uint32_t PM_factory::max_sample_depth() const noexcept {
    return 1;
}

}  // namespace rendering::integrator::surface