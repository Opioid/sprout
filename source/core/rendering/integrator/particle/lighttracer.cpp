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
      light_sampler_(rng),
      material_samplers_{rng, rng, rng} {}

Lighttracer::~Lighttracer() noexcept {}

void Lighttracer::prepare(Scene const& /*scene*/, uint32_t num_samples_per_pixel) noexcept {
    sampler_.resize(num_samples_per_pixel, 1, 1, 1);

    light_sampler_.resize(num_samples_per_pixel, 1, 2, 3);

    for (auto& s : material_samplers_) {
        s.resize(num_samples_per_pixel, 1, 1, 1);
    }
}

void Lighttracer::start_pixel() noexcept {
    sampler_.start_pixel();

    light_sampler_.start_pixel();

    for (auto& s : material_samplers_) {
        s.start_pixel();
    }
}

void Lighttracer::li(uint32_t frame, int4 const& bounds, Worker& worker,
                     Interface_stack const& /*initial_stack*/) noexcept {
    worker.interface_stack().clear();

    Camera const& camera = worker.camera();

    Prop const* camera_prop = worker.scene().prop(camera.entity());

    Filter const filter = Filter::Undefined;

    Bxdf_sample sample_result;

    float3 throughput(1.f);

    bool const avoid_caustics = false;

    Intersection intersection;

    Sample_from light_sample;

    bool caustic_ray = false;

    Ray   ray;
    Light light;
    if (!generate_light_ray(frame, worker, ray, light, light_sample)) {
        return;
    }

    if (!worker.intersect_and_resolve_mask(ray, intersection, filter)) {
        return;
    }

    int2 const d = camera.sensor().dimensions();

    float const weight = static_cast<float>(d[0] * d[1]) / settings_.num_light_paths;

    float3 radiance = weight * light.evaluate(light_sample, Filter::Nearest, worker) /
                      (light_sample.pdf);

    for (;;) {
        float3 const wo = -ray.direction;

        auto const& material_sample = intersection.sample(wo, ray, filter, avoid_caustics, sampler_,
                                                          worker);

        if (material_sample.is_pure_emissive()) {
            break;
        }

        material_sample.sample(material_sampler(ray.depth), sample_result);
        if (0.f == sample_result.pdf) {
            break;
        }

        if (material_sample.ior_greater_one()) {
            if (sample_result.type.test(Bxdf_type::Caustic)) {
                caustic_ray = true;
            } else {
                if ((intersection.subsurface || material_sample.same_hemisphere(wo)) &&
                    ((caustic_ray &&
                      worker.interface_stack().top_is_vacuum_or_not_scattering(worker)) ||
                     settings_.full_light_path)) {
                    direct_camera(camera, camera_prop, bounds, radiance, ray, intersection.geo.p,
                                  material_sample, filter, worker);

                    if (!settings_.indirect_caustics) {
                        break;
                    }
                }
            }
        }

        if (ray.depth >= settings_.max_bounces - 1) {
            break;
        }

        if (0.f == ray.wavelength) {
            ray.wavelength = sample_result.wavelength;
        }

        if (material_sample.ior_greater_one()) {
            //   transparent &= sample_result.type.test(Bxdf_type::Pass_through);

            radiance *= sample_result.reflection / sample_result.pdf;

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

            //   result += throughput * vli;
            throughput *= vtr;

            if (Event::Abort == hit || Event::Absorb == hit) {
                break;
            }
        } else if (!worker.intersect_and_resolve_mask(ray, intersection, filter)) {
            break;
        }
    }
}

bool Lighttracer::generate_light_ray(uint32_t frame, Worker& worker, Ray& ray, Light& light_out,
                                     Sample_from& light_sample) noexcept {
    Scene const& scene = worker.scene();

    float const select = light_sampler_.generate_sample_1D(1);

    auto const light = scene.random_light(select);

    uint64_t const time = worker.absolute_time(frame, light_sampler_.generate_sample_1D(2));

    if (!light.ref.sample(time, light_sampler_, 1, scene.aabb(), worker, light_sample)) {
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

void Lighttracer::direct_camera(Camera const& camera, Prop const* camera_prop, int4 const& bounds,
                                float3 const& radiance, Ray const& history, float3 const& p,
                                Material_sample const& material_sample, Filter filter,
                                Worker& worker) const noexcept {
    Camera_sample_to camera_sample;
    if (!camera.sample(camera_prop, history.time, p, worker.scene(), camera_sample)) {
        return;
    }

    Ray const ray(material_sample.offset_p(p), -camera_sample.dir, 0.f, camera_sample.t,
                  history.depth, history.time, history.wavelength);

    float mv;
    if (worker.masked_visibility(ray, filter, mv)) {
        float3 const wi   = -camera_sample.dir;
        auto const   bxdf = material_sample.evaluate_f(wi, true);

        auto& sensor = camera.sensor();

        float3 const result = camera_sample.pdf * radiance * bxdf.reflection;

        sensor.splat_sample(camera_sample, float4(result, 1.f), bounds);
    }
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
                                         uint32_t max_bounces, uint32_t num_light_paths,
                                         bool indirect_caustics, bool full_light_path) noexcept
    : take_settings_(take_settings),
      integrators_(memory::allocate_aligned<Lighttracer>(num_integrators)),
      settings_{min_bounces, max_bounces, static_cast<float>(num_light_paths), indirect_caustics,
                full_light_path} {}

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
