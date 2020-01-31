#include "lighttracer.hpp"
#include "base/math/frustum.hpp"
#include "base/math/vector3.inl"
#include "base/memory/align.hpp"
#include "base/random/generator.inl"
#include "base/spectrum/rgb.hpp"
#include "rendering/integrator/integrator_helper.hpp"
#include "rendering/integrator/particle/particle_importance.hpp"
#include "rendering/rendering_worker.inl"
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

Lighttracer::Lighttracer(rnd::Generator& rng, Settings const& settings) noexcept
    : Integrator(rng),
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

    Importance_cache& importance = worker.particle_importance();

    Filter const filter = Filter::Undefined;

    Bxdf_sample sample_result;

    bool const avoid_caustics = false;

    Intersection intersection;

    Sample_from light_sample;

    bool caustic_ray = false;

    // ---
    // Frustum const frustum = worker.camera().frustum();

    AABB const& world_bounds = settings_.full_light_path ? worker.scene().aabb()
                                                         : worker.scene().caustic_aabb();

    AABB const frustum_bounds = world_bounds;  //.intersection(frustum.calculate_aabb());
    // ---

    Ray          ray;
    Light const* light;
    uint32_t     light_id;
    if (!generate_light_ray(frame, frustum_bounds, worker, ray, light, light_id, light_sample)) {
        return;
    }

    if (!worker.intersect_and_resolve_mask(ray, intersection, filter)) {
        return;
    }

    float3 radiance = light->evaluate(light_sample, Filter::Nearest, worker) / (light_sample.pdf);

    bool first = true;

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
            if (sample_result.type.is(Bxdf_type::Caustic)) {
                caustic_ray = true;
            } else {
                bool const side = intersection.subsurface | material_sample.same_hemisphere(wo);

                bool const caustic = caustic_ray & worker.interface_stack().top_is_straight(worker);

                if (side & (caustic | settings_.full_light_path)) {
                    if (direct_camera(camera, bounds, radiance, ray, intersection, material_sample,
                                      filter, worker)) {
                        if (first) {
                            importance.increment(light_id, light_sample.xy, intersection.geo.p);
                            first = false;
                        }
                    }
                }

                if (!settings_.indirect_caustics) {
                    break;
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
            //   transparent &= sample_result.type.test(Bxdf_type::Straight);

            radiance *= sample_result.reflection / sample_result.pdf;

            ray.origin = material_sample.offset_p(intersection.geo.p, sample_result.wi);
            ray.min_t  = 0.f;
            ray.set_direction(sample_result.wi);
            ++ray.depth;
        } else {
            ray.min_t = scene::offset_f(ray.max_t);
        }

        ray.max_t = scene::Ray_max_t;

        if (sample_result.type.is(Bxdf_type::Transmission)) {
            auto const ior = worker.interface_change_ior(sample_result.wi, intersection);

            float const eta = ior.eta_i / ior.eta_t;

            radiance *= eta * eta;
        }

        if (!worker.interface_stack().empty()) {
            float3     vli;
            float3     vtr;
            auto const hit = worker.volume(ray, intersection, filter, vli, vtr);

            //   result += throughput * vli;
            radiance *= vtr;

            if ((Event::Abort == hit) | (Event::Absorb == hit)) {
                break;
            }
        } else if (!worker.intersect_and_resolve_mask(ray, intersection, filter)) {
            break;
        }
    }
}

bool Lighttracer::generate_light_ray(uint32_t frame, AABB const& bounds, Worker& worker, Ray& ray,
                                     Light const*& light_out, uint32_t& light_id,
                                     Sample_from& light_sample) noexcept {
    float const select = light_sampler_.generate_sample_1D(1);

    auto const light = worker.scene().random_light(select);

    uint64_t const time = worker.absolute_time(frame, light_sampler_.generate_sample_1D(2));

    Importance const& importance = worker.particle_importance().importance(light.id);

    if (importance.distribution().empty()) {
        if (!light.ref.sample(time, light_sampler_, 1, bounds, worker, light_sample)) {
            return false;
        }
    } else {
        if (!light.ref.sample(time, light_sampler_, 1, importance.distribution(), bounds, worker,
                              light_sample)) {
            return false;
        }

        light_sample.pdf *= importance.denormalization_factor();
    }

    ray.origin = scene::offset_ray(light_sample.p, light_sample.dir);
    ray.set_direction(light_sample.dir);
    ray.min_t      = 0.f;
    ray.max_t      = scene::Ray_max_t;
    ray.depth      = 0;
    ray.time       = time;
    ray.wavelength = 0.f;

    light_out = &light.ref;
    light_id  = light.id;

    light_sample.pdf *= light.pdf;

    return true;
}

bool Lighttracer::direct_camera(Camera const& camera, int4 const& bounds, float3 const& radiance,
                                Ray const& history, Intersection const& intersection,
                                Material_sample const& material_sample, Filter filter,
                                Worker& worker) noexcept {
    if (!worker.scene().prop(intersection.prop)->visible_in_camera()) {
        return false;
    }

    float3 const p = material_sample.offset_p(intersection.geo.p);

    Camera_sample_to camera_sample;
    if (!camera.sample(bounds, history.time, p, sampler_, 0, worker.scene(), camera_sample)) {
        return false;
    }

    Ray ray(p, -camera_sample.dir, 0.f, camera_sample.t, history.depth, history.time,
            history.wavelength);

    float3 tr;
    if (!worker.transmitted(ray, material_sample.wo(), intersection, filter, tr)) {
        return false;
    }

    float3 const wi   = -camera_sample.dir;
    auto const   bxdf = material_sample.evaluate_f(wi, true);

    auto& sensor = camera.sensor();

    float3 const wo = material_sample.wo();

    float3 const& n = material_sample.base_shading_normal();

    float const nsc = non_symmetry_compensation(wo, wi, intersection.geo.geo_n, n);

    float3 const result = (camera_sample.pdf * nsc) * (tr * radiance * bxdf.reflection);

    sensor.splat_sample(camera_sample, float4(result, 1.f), bounds);

    return true;
}

sampler::Sampler& Lighttracer::material_sampler(uint32_t bounce) noexcept {
    if (Num_material_samplers > bounce) {
        return material_samplers_[bounce];
    }

    return sampler_;
}

Lighttracer_pool::Lighttracer_pool(uint32_t num_integrators, uint32_t min_bounces,
                                   uint32_t max_bounces, bool indirect_caustics,
                                   bool full_light_path) noexcept
    : num_integrators_(num_integrators),
      integrators_(memory::allocate_aligned<Lighttracer>(num_integrators)),
      settings_{min_bounces, max_bounces, indirect_caustics, full_light_path} {
    std::memset(reinterpret_cast<void*>(integrators_), 0, sizeof(Lighttracer) * num_integrators);
}

Lighttracer_pool::~Lighttracer_pool() noexcept {
    for (uint32_t i = 0, len = num_integrators_; i < len; ++i) {
        memory::destroy(&integrators_[i]);
    }

    memory::free_aligned(integrators_);
}

Lighttracer* Lighttracer_pool::get(uint32_t id, rnd::Generator& rng) const noexcept {
    if (uint32_t const zero = 0;
        0 == std::memcmp(&zero, reinterpret_cast<void*>(&integrators_[id]), 4)) {
        return new (&integrators_[id]) Lighttracer(rng, settings_);
    }

    return &integrators_[id];
}

uint32_t Lighttracer_pool::max_sample_depth() const noexcept {
    return 1;
}

}  // namespace rendering::integrator::particle
