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

void Lighttracer::li(uint32_t frame, int4 const& bounds, Worker& worker,
                     Interface_stack const& /*initial_stack*/) noexcept {
    worker.interface_stack().clear();

    scene::camera::Camera const& camera = worker.camera();

    scene::prop::Prop const* camera_prop = worker.scene().prop(camera.entity());

    Filter const filter = Filter::Undefined;

    bool const avoid_caustics = false;

    Intersection intersection;

    Sample_from light_sample;

    Ray   ray;
    Light light;
    if (!generate_light_ray(frame, worker, ray, light, light_sample)) {
        return;
    }

    if (!worker.intersect_and_resolve_mask(ray, intersection, filter)) {
        return;
    }

    Camera_sample_to camera_sample;

    camera.sample(camera_prop, ray.time, intersection.geo.p, worker.scene(), camera_sample);

    float const t = distance(intersection.geo.p, camera_sample.p);

    Ray visibility_ray;
    visibility_ray.origin = intersection.geo.p;
    visibility_ray.set_direction(-camera_sample.dir);
    visibility_ray.min_t = scene::offset_f(0.f);
    visibility_ray.max_t = t;
    visibility_ray.depth = ray.depth;
    visibility_ray.time  = ray.time;

    float mv;
    if (!worker.masked_visibility(visibility_ray, filter, mv)) {
        return;
    }

    float3 radiance = light.evaluate(light_sample, Filter::Nearest, worker) / (light_sample.pdf);

    float3 const wo = -ray.direction;

    auto const& material_sample = intersection.sample(wo, ray, filter, avoid_caustics, sampler_,
                                                      worker);

    float3 const wi   = -camera_sample.dir;
    auto const   bxdf = material_sample.evaluate_b(wi, true);

    auto& sensor = camera.sensor();

    sensor.add_sample(camera_sample, float4(radiance * bxdf.reflection, 1.f), bounds);
}

bool Lighttracer::generate_light_ray(uint32_t frame, Worker& worker, Ray& ray, Light& light_out,
                                     Sample_from& light_sample) noexcept {
    Scene const& scene = worker.scene();

    float const select = sampler_.generate_sample_1D(1);

    auto const light = scene.random_light(select);

    uint64_t const time = worker.absolute_time(frame, sampler_.generate_sample_1D(2));

    if (!light.ref.sample(time, sampler_, 0, scene.aabb(), worker, light_sample)) {
        return false;
    }

    ray.origin = scene::offset_ray(light_sample.p, light_sample.dir);
    ray.set_direction(light_sample.dir);
    ray.min_t      = 0.f;
    ray.max_t      = scene::Ray_max_t;
    ray.depth      = 0;
    ray.time       = time;
    ray.wavelength = 0.f;

    light_out = light.ref;

    light_sample.pdf *= light.pdf;

    return true;
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
