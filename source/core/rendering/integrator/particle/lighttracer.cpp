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
#include "scene/material/material.inl"
#include "scene/material/material_helper.hpp"
#include "scene/material/material_sample.inl"
#include "scene/prop/interface_stack.inl"
#include "scene/prop/prop_intersection.inl"
#include "scene/scene.hpp"
#include "scene/scene_constants.hpp"
#include "scene/scene_ray.inl"
#include "scene/shape/shape_sample.hpp"

namespace rendering::integrator::particle {

using namespace scene;

Lighttracer::Lighttracer(Settings const& settings) : settings_(settings) {}

Lighttracer::~Lighttracer() = default;

void Lighttracer::prepare(Scene const& /*scene*/, uint32_t num_samples_per_pixel) {
    sampler_.resize(num_samples_per_pixel, 1, 1, 1);

    light_sampler_.resize(num_samples_per_pixel, 1, 2, 3);

    for (auto& s : material_samplers_) {
        s.resize(num_samples_per_pixel, 1, 1, 1);
    }
}

void Lighttracer::start_pixel(rnd::Generator& rng) {
    sampler_.start_pixel(rng);

    light_sampler_.start_pixel(rng);

    for (auto& s : material_samplers_) {
        s.start_pixel(rng);
    }
}

void Lighttracer::li(uint32_t frame, Worker& worker, Interface_stack const& /*initial_stack*/) {
    worker.interface_stack().clear();

    // ---
    // Frustum const frustum = worker.camera().frustum();

    AABB const world_bounds = settings_.full_light_path ? worker.scene().aabb()
                                                        : worker.scene().caustic_aabb();

    AABB const frustum_bounds = world_bounds;  //.intersection(frustum.calculate_aabb());
    // ---

    Ray          ray;
    Light const* light;
    uint32_t     light_id;
    Sample_from  light_sample;
    if (!generate_light_ray(frame, frustum_bounds, worker, ray, light, light_id, light_sample)) {
        return;
    }

    Intersection intersection;

    Filter const filter = Filter::Undefined;

    if (!worker.intersect_and_resolve_mask(ray, intersection, filter)) {
        return;
    }

    Camera const& camera = worker.camera();

    Importance_cache& importance = worker.particle_importance();

    float3 radiance = light->evaluate(light_sample, Filter::Nearest, worker) / (light_sample.pdf);
    float3 wo1(0.f);

    Bxdf_sample sample_result;

    bool const avoid_caustics = false;

    bool primary_ray     = true;
    bool caustic_path    = false;
    bool from_subsurface = false;
    bool first           = true;

    for (;;) {
        float3 const wo = -ray.direction;

        auto const& material_sample = worker.sample_material(
            ray, wo, wo1, intersection, filter, avoid_caustics, from_subsurface, sampler_);

        wo1 = wo;

        if (material_sample.is_pure_emissive()) {
            break;
        }

        material_sample.sample(material_sampler(ray.depth), worker.rng(), sample_result);
        if (0.f == sample_result.pdf) {
            break;
        }

        if (sample_result.type.no(Bxdf_type::Straight)) {
            if (sample_result.type.no(Bxdf_type::Specular)) {
                bool const side = intersection.subsurface | material_sample.same_hemisphere(wo);

                if (side & (caustic_path | settings_.full_light_path)) {
                    if (direct_camera(camera, radiance, ray, intersection, material_sample, filter,
                                      worker)) {
                        if (first) {
                            importance.increment(light_id, light_sample.xy, intersection.geo.p);
                            first = false;
                        }
                    }
                }
            }

            if (sample_result.type.is(Bxdf_type::Caustic)) {
                //   caustic_path |= !primary_ray;//true;
                caustic_path = true;
            } else {
                primary_ray = false;
            }
        }

        if (ray.depth >= settings_.max_bounces - 1) {
            break;
        }

        if (0.f == ray.wavelength) {
            ray.wavelength = sample_result.wavelength;
        }

        radiance *= sample_result.reflection / sample_result.pdf;

        if (sample_result.type.is(Bxdf_type::Straight)) {
            ray.min_t() = scene::offset_f(ray.max_t());

            if (sample_result.type.no(Bxdf_type::Transmission)) {
                ++ray.depth;
            }
        } else {
            ray.origin = material_sample.offset_p(intersection.geo.p, sample_result.wi,
                                                  intersection.subsurface);
            ray.set_direction(sample_result.wi);
            ++ray.depth;

            from_subsurface = false;
        }

        ray.max_t() = scene::Ray_max_t;

        if (sample_result.type.is(Bxdf_type::Transmission)) {
            auto const ior = worker.interface_change_ior(sample_result.wi, intersection);

            float const eta = ior.eta_i / ior.eta_t;

            radiance *= eta * eta;
        }

        from_subsurface |= intersection.subsurface;

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
                                     Sample_from& light_sample) {
    auto& rng = worker.rng();

    float const select = light_sampler_.generate_sample_1D(rng, 1);

    auto const light = worker.scene().random_light(select);

    uint64_t const time = worker.absolute_time(frame, light_sampler_.generate_sample_1D(rng, 2));

    Importance const& importance = worker.particle_importance().importance(light.id);

    if (importance.distribution().empty()) {
        if (!light.ptr->sample(time, light_sampler_, 1, bounds, worker, light_sample)) {
            return false;
        }
    } else {
        if (!light.ptr->sample(time, light_sampler_, 1, importance.distribution(), bounds, worker,
                              light_sample)) {
            return false;
        }

        light_sample.pdf *= importance.denormalization_factor();
    }

    ray.origin = scene::offset_ray(light_sample.p, light_sample.dir);
    ray.set_direction(light_sample.dir);
    ray.min_t()    = 0.f;
    ray.max_t()    = scene::Ray_max_t;
    ray.depth      = 0;
    ray.time       = time;
    ray.wavelength = 0.f;

    light_out = light.ptr;
    light_id  = light.id;

    light_sample.pdf *= light.pdf;

    return true;
}

bool Lighttracer::direct_camera(Camera const& camera, float3 const& radiance, Ray const& history,
                                Intersection const&    intersection,
                                Material_sample const& material_sample, Filter filter,
                                Worker& worker) {
    if (!intersection.visible_in_camera(worker)) {
        return false;
    }

    auto& sensor = camera.sensor();

    int32_t const fr = camera.sensor().filter_radius_int();

    int4 filter_crop = camera.crop() + int4(-fr, -fr, fr, fr);
    filter_crop[2] -= filter_crop[0] + 1;
    filter_crop[3] -= filter_crop[1] + 1;

    bool hit = false;

    float3 const p = material_sample.offset_p(intersection.geo.p, intersection.subsurface, false);

    for (uint32_t v = 0, len = camera.num_views(); v < len; ++v) {
        Camera_sample_to camera_sample;
        if (!camera.sample(v, filter_crop, history.time, p, sampler_, worker.rng(), 0,
                           worker.scene(), camera_sample)) {
            continue;
        }

        Ray ray(p, -camera_sample.dir, 0.f, camera_sample.t, history.depth, history.wavelength,
                history.time);

        float3 tr;
        if (!worker.transmitted(ray, material_sample.wo(), intersection, filter, tr)) {
            continue;
        }

        float3 const wi = -camera_sample.dir;

        auto const bxdf = material_sample.evaluate_f(wi);

        float3 const& wo = material_sample.wo();

        float3 const& n = material_sample.interpolated_normal();

        float nsc = material::non_symmetry_compensation(wo, wi, intersection.geo.geo_n, n);

        auto const& material = *intersection.material(worker);

        if (intersection.subsurface && (material.ior() > 1.f)) {
            float const ior_t = worker.interface_stack().next_to_bottom_ior(worker);
            nsc *= material.ior() / ior_t;
        }

        float3 const result = (camera_sample.pdf * nsc) * (tr * radiance * bxdf.reflection);

        int2 const offset = camera.view_offset(v);

        int4 crop = camera.crop();
        crop[2] -= crop[0] + 1;
        crop[3] -= crop[1] + 1;
        crop[0] += offset[0];
        crop[1] += offset[1];

        sensor.splat_sample(camera_sample, float4(result, 1.f), offset, crop);

        hit = true;
    }

    return hit;
}

sampler::Sampler& Lighttracer::material_sampler(uint32_t bounce) {
    if (Num_material_samplers > bounce) {
        return material_samplers_[bounce];
    }

    return sampler_;
}

Lighttracer_pool::Lighttracer_pool(uint32_t num_integrators, uint32_t min_bounces,
                                   uint32_t max_bounces, bool full_light_path)
    : num_integrators_(num_integrators),
      integrators_(memory::allocate_aligned<Lighttracer>(num_integrators)),
      settings_{min_bounces, max_bounces, full_light_path} {
    std::memset(static_cast<void*>(integrators_), 0, sizeof(Lighttracer) * num_integrators);
}

Lighttracer_pool::~Lighttracer_pool() {
    for (uint32_t i = 0, len = num_integrators_; i < len; ++i) {
        memory::destroy(&integrators_[i]);
    }

    memory::free_aligned(integrators_);
}

Lighttracer* Lighttracer_pool::get(uint32_t id) const {
    if (uint32_t const zero = 0;
        0 == std::memcmp(&zero, static_cast<void*>(&integrators_[id]), 4)) {
        return new (&integrators_[id]) Lighttracer(settings_);
    }

    return &integrators_[id];
}

uint32_t Lighttracer_pool::max_sample_depth() const {
    return 1;
}

}  // namespace rendering::integrator::particle
