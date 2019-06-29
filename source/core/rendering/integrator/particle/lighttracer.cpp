#include "lighttracer.hpp"
#include "base/math/vector3.inl"
#include "base/memory/align.hpp"
#include "base/random/generator.inl"
#include "base/spectrum/rgb.hpp"
#include "rendering/integrator/integrator_helper.hpp"
#include "rendering/rendering_worker.hpp"
#include "rendering/sensor/sensor.hpp"
#include "sampler/camera_sample.hpp"
#include "scene/camera/camera.hpp"
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

namespace rendering::integrator::particle {

Lighttracer::Lighttracer(rnd::Generator& rng, take::Settings const& take_settings,
                         Settings const& settings) noexcept
    : Integrator(rng, take_settings),
      settings_(settings),
      sampler_(rng),
      material_samplers_{rng, rng, rng} {}

Lighttracer::~Lighttracer() noexcept {}

void Lighttracer::prepare(Scene const& /*scene*/, uint32_t num_samples_per_pixel) noexcept {
    sampler_.resize(num_samples_per_pixel, 1, 1, 1);

    for (auto& s : material_samplers_) {
        s.resize(num_samples_per_pixel, 1, 1, 1);
    }
}

void Lighttracer::start_pixel() noexcept {
    sampler_.start_pixel();

    for (auto& s : material_samplers_) {
        s.start_pixel();
    }
}

void Lighttracer::li(int4 const& bounds, Worker& worker,
                     Interface_stack const& /*initial_stack*/) noexcept {
    scene::camera::Camera const& camera = worker.camera();

    //   scene::prop::Prop const* camera_prop = scene_->prop(camera.entity());

    auto& sensor = camera.sensor();

    Camera_sample camera_sample;

    camera_sample.pixel    = int2(4, 4);
    camera_sample.pixel_uv = float2(0.5);

    sensor.add_sample(camera_sample, float4(1.f, 0.f, 0.f, 1.f), bounds);
}

bool Lighttracer::generate_light_ray(uint64_t time, Worker& worker, Ray& ray,
                                     float3& radiance) noexcept {
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

float3 Lighttracer::direct_light(Ray const& ray, Intersection const& intersection,
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

sampler::Sampler& Lighttracer::material_sampler(uint32_t bounce) noexcept {
    if (Num_material_samplers > bounce) {
        return material_samplers_[bounce];
    }

    return sampler_;
}

size_t Lighttracer::num_bytes() const noexcept {
    size_t sampler_bytes = 0;

    for (auto& s : material_samplers_) {
        sampler_bytes += s.num_bytes();
    }

    return sizeof(*this) + sampler_.num_bytes() + sampler_bytes;
}

Lighttracer_factory::Lighttracer_factory(take::Settings const& take_settings,
                                         uint32_t num_integrators, uint32_t min_bounces,
                                         uint32_t max_bounces) noexcept
    : take_settings_(take_settings),
      integrators_(memory::allocate_aligned<Lighttracer>(num_integrators)),
      settings_{min_bounces, max_bounces} {}

Lighttracer_factory::~Lighttracer_factory() noexcept {
    memory::free_aligned(integrators_);
}

Lighttracer* Lighttracer_factory::create(uint32_t id, rnd::Generator& rng) const noexcept {
    return new (&integrators_[id]) Lighttracer(rng, take_settings_, settings_);
}

uint32_t Lighttracer_factory::max_sample_depth() const noexcept {
    return 1;
}

}  // namespace rendering::integrator::particle
