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

Lighttracer::Lighttracer(Settings const& settings, uint32_t /*max_samples_per_pixel*/)
    : settings_(settings) {}

Lighttracer::~Lighttracer() = default;

void Lighttracer::start_pixel(RNG& rng) {
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

    AABB const frustum_bounds = world_bounds;  //.isec(frustum.calculate_aabb());
    // ---

    Ray         ray;
    uint32_t    light_id;
    Sample_from light_sample;
    if (!generate_light_ray(frame, frustum_bounds, worker, ray, light_id, light_sample)) {
        return;
    }

    Intersection isec;

    Filter filter = Filter::Undefined;

    if (!worker.intersect_and_resolve_mask(ray, isec, filter)) {
        return;
    }

    Camera const& camera = worker.camera();

    Importance_cache& importance = worker.particle_importance();

    auto const& light = worker.scene().light(light_id);

    float3 radiance = light.evaluate(light_sample, Filter::Nearest, worker) / (light_sample.pdf);
    float3 wo1(0.f);

    Bxdf_sample sample_result;

    bool const avoid_caustics = false;

    bool primary_ray     = true;
    bool caustic_path    = false;
    bool from_subsurface = false;
    bool first           = true;

    for (;;) {
        float3 const wo = -ray.direction;

        auto const& mat_sample = worker.sample_material(ray, wo, wo1, isec, filter, 0.f,
                                                        avoid_caustics, from_subsurface, sampler_);

        wo1 = wo;

        if (mat_sample.is_pure_emissive()) {
            break;
        }

        mat_sample.sample(material_sampler(ray.depth), worker.rng(), sample_result);
        if (0.f == sample_result.pdf) {
            break;
        }

        if (sample_result.type.no(Bxdf_type::Straight)) {
            if (sample_result.type.no(Bxdf_type::Specular) &&
                (isec.subsurface | mat_sample.same_hemisphere(wo)) &&
                (caustic_path | settings_.full_light_path)) {
                if (direct_camera(camera, radiance, ray, isec, mat_sample, filter, worker)) {
                    if (first) {
                        importance.increment(light_id, light_sample.xy, isec.geo.p);
                        first = false;
                    }
                }
            }

            if (sample_result.type.is(Bxdf_type::Caustic)) {
                //   caustic_path |= !primary_ray;//true;
                caustic_path = true;
            } else {
                filter      = Filter::Nearest;
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
            ray.origin = mat_sample.offset_p(isec.geo.p, sample_result.wi, isec.subsurface);
            ray.set_direction(sample_result.wi);
            ++ray.depth;

            from_subsurface = false;
        }

        ray.max_t() = scene::Ray_max_t;

        if (sample_result.type.is(Bxdf_type::Transmission)) {
            auto const ior = worker.interface_change_ior(sample_result.wi, isec);

            float const eta = ior.eta_i / ior.eta_t;

            radiance *= eta * eta;
        }

        from_subsurface |= isec.subsurface;

        if (!worker.interface_stack().empty()) {
            float3     vli;
            float3     vtr;
            auto const hit = worker.volume(ray, isec, filter, vli, vtr);

            //   result += throughput * vli;
            radiance *= vtr;

            if ((Event::Abort == hit) | (Event::Absorb == hit)) {
                break;
            }
        } else if (!worker.intersect_and_resolve_mask(ray, isec, filter)) {
            break;
        }
    }
}

bool Lighttracer::generate_light_ray(uint32_t frame, AABB const& bounds, Worker& worker, Ray& ray,
                                     uint32_t& light_id, Sample_from& light_sample) {
    if (0 == worker.scene().num_lights()) {
        return false;
    }

    auto& rng = worker.rng();

    float const select = light_sampler_.sample_1D(rng, 1);

    auto const  light     = worker.scene().random_light(select);
    auto const& light_ref = worker.scene().light(light.id);

    uint64_t const time = worker.absolute_time(frame, light_sampler_.sample_1D(rng, 2));

    Importance const& importance = worker.particle_importance().importance(light.id);

    if (importance.distribution().empty()) {
        if (!light_ref.sample(time, light_sampler_, 1, bounds, worker, light_sample)) {
            return false;
        }
    } else {
        if (!light_ref.sample(time, light_sampler_, 1, importance.distribution(), bounds, worker,
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

    light_id = light.id;

    light_sample.pdf *= light.pdf;

    return true;
}

bool Lighttracer::direct_camera(Camera const& camera, float3_p radiance, Ray const& history,
                                Intersection const& isec, Material_sample const& mat_sample,
                                Filter filter, Worker& worker) {
    if (!isec.visible_in_camera(worker)) {
        return false;
    }

    auto& sensor = camera.sensor();

    int32_t const fr = camera.sensor().filter_radius_int();

    int4 filter_crop = camera.crop() + int4(-fr, -fr, fr, fr);
    filter_crop[2] -= filter_crop[0] + 1;
    filter_crop[3] -= filter_crop[1] + 1;

    bool hit = false;

    float3 const p = mat_sample.offset_p(isec.geo.p, isec.subsurface, false);

    for (uint32_t v = 0, len = camera.num_views(); v < len; ++v) {
        Camera_sample_to camera_sample;
        if (!camera.sample(v, filter_crop, history.time, p, sampler_, worker.rng(), 0,
                           worker.scene(), camera_sample)) {
            continue;
        }

        Ray ray(p, -camera_sample.dir, 0.f, camera_sample.t, history.depth, history.wavelength,
                history.time);

        float3 tr;
        if (!worker.transmitted(ray, mat_sample.wo(), isec, filter, tr)) {
            continue;
        }

        float3 const wi = -camera_sample.dir;

        auto const bxdf = mat_sample.evaluate(wi);

        float3 const wo = mat_sample.wo();

        float3 const n = mat_sample.interpolated_normal();

        float nsc = material::non_symmetry_compensation(wo, wi, isec.geo.geo_n, n);

        auto const& material = *isec.material(worker);

        if (isec.subsurface && (material.ior() > 1.f)) {
            float const ior_t = worker.interface_stack().next_to_bottom_ior(worker);
            float const eta   = material.ior() / ior_t;

            nsc *= eta * eta;
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
      settings_{min_bounces, max_bounces, full_light_path} {}

Lighttracer_pool::~Lighttracer_pool() {
    for (uint32_t i = 0, len = num_integrators_; i < len; ++i) {
        integrators_[i].~Lighttracer();
    }

    std::free(integrators_);
}

Lighttracer* Lighttracer_pool::create(uint32_t id, uint32_t max_samples_per_pixel) const {
    return new (&integrators_[id]) Lighttracer(settings_, max_samples_per_pixel);
}

uint32_t Lighttracer_pool::max_sample_depth() const {
    return 1;
}

}  // namespace rendering::integrator::particle
