#include "aerial_perspective.hpp"
#include "base/math/aabb.inl"
#include "base/math/sampling/sampling.hpp"
#include "base/math/vector3.inl"
#include "base/memory/align.hpp"
#include "base/random/generator.inl"
#include "rendering/rendering_worker.hpp"
#include "scene/light/light.hpp"
#include "scene/light/light_sample.hpp"
#include "scene/material/bxdf.hpp"
#include "scene/material/material_sample.inl"
#include "scene/prop/prop_intersection.inl"
#include "scene/scene.hpp"
#include "scene/scene_constants.hpp"
#include "scene/scene_ray.inl"
#include "scene/scene_renderstate.hpp"
#include "scene/shape/shape.hpp"
#include "tracking.hpp"

namespace rendering::integrator::volume {

Aerial_perspective::Aerial_perspective(rnd::Generator& rng, take::Settings const& take_settings,
                                       Settings const& settings)
    : Integrator(rng, take_settings), settings_(settings), sampler_(rng) {}

void Aerial_perspective::prepare(Scene const& /*scene*/, uint32_t num_samples_per_pixel) {
    sampler_.resize(num_samples_per_pixel, 1, 1, 1);
}

void Aerial_perspective::resume_pixel(uint32_t /*sample*/, rnd::Generator& /*scramble*/) {}

/*
float3 Aerial_perspective::li(Ray const& ray, const Volume& volume,
                                                          Worker& worker, float3& transmittance) {
        if (ray.properties.test(Ray::Property::Recursive)) {
                transmittance = Aerial_perspective::transmittance(ray, volume, worker);
                return float3::identity();
        }

        if (settings_.disable_shadows) {
                return integrate_without_shadows(ray, volume, worker, transmittance);
        }

        return integrate_with_shadows(ray, volume, worker, transmittance);
}
*/

float3 Aerial_perspective::transmittance(Ray const& ray, Worker& worker) {
    return Tracking::transmittance(ray, rng_, worker);
}

bool Aerial_perspective::integrate(Ray& /*ray*/, Intersection& /*intersection*/,
                                   Sampler_filter /*filter*/, Worker& /*worker*/, float3& /*li*/,
                                   float3& /*transmittance*/) {
    return false;
}

size_t Aerial_perspective::num_bytes() const {
    return sizeof(*this) + sampler_.num_bytes();
}

/*
float3 Aerial_perspective::integrate_with_shadows(Ray const& ray, const Volume& volume,
                                                                                                  Worker&
worker, float3& transmittance) { float min_t = ray.min_t; float const range = ray.max_t - min_t;

        Transformation temp;
        auto const& transformation = volume.transformation_at(ray.time, temp);

        uint32_t const max_samples = static_cast<uint32_t>(std::ceil(range / settings_.step_size));
        uint32_t const num_samples = ray.is_primary() ? max_samples : 1;

        float const step = range / static_cast<float>(num_samples);

        float3 radiance(0.f);
        float3 tr(1.f);

        float3 const start = ray.point(min_t);

        float const r = std::max(rng_.random_float(), 0.0001f);

        min_t += r * step;

        float3 const next = ray.point(min_t);
        Ray tau_ray(start, next - start, 0.f, 1.f, 0, ray.time);

        float3 const tau_ray_direction     = ray.point(min_t + step) - next;
        float3 const inv_tau_ray_direction = math::reciprocal(tau_ray_direction);

        auto const& material = *volume.material(0);

        for (uint32_t i = num_samples; i > 0; --i, min_t += step) {
                // This happens sometimes when the range is very small compared to the world
coordinates. if (float3::identity() == tau_ray.direction) { tau_ray.origin = ray.point(min_t +
step); tau_ray.direction = tau_ray_direction; tau_ray.inv_direction = inv_tau_ray_direction;
                        continue;
                }

                float3 const tau = material.optical_depth(transformation, volume.aabb(), tau_ray,
                                                                                                  settings_.step_size,
rng_, Sampler_filter::Undefined, worker);

                tr *= math::exp(-tau);

                float3 const current = ray.point(min_t);
                tau_ray.origin = current;
                // This stays the same during the loop,
                // but we need a different value in the first iteration.
                // Would be nicer to restructure the loop.
                tau_ray.direction = tau_ray_direction;
                tau_ray.inv_direction = inv_tau_ray_direction;

                // Lighting
                Ray secondary_ray = ray;
                secondary_ray.properties.set(Ray::Property::Recursive);
                secondary_ray.set_primary(false);
                secondary_ray.depth = 0xFFFFFFFE;

                scene::prop::Intersection secondary_intersection;
                secondary_intersection.prop = &volume;
                secondary_intersection.geo.p = current;
                secondary_intersection.geo.part = 0;
                secondary_intersection.geo.epsilon = 0.0005f;

                float3 const local_radiance = worker.li(secondary_ray, secondary_intersection);

                float3 mu_a, scattering;
                material.collision_coefficients(transformation, current,
                                                        Sampler_filter::Undefined, worker, mu_a,
scattering);

                radiance += tr * scattering * local_radiance;
        }

        transmittance = tr;

        float3 const color = step * radiance;

        return color;
}

float3 Aerial_perspective::integrate_without_shadows(Ray const& ray, const Volume& volume,
                                                                                                         Worker& worker, float3& transmittance) {
        float min_t = ray.min_t;
        float const range = ray.max_t - min_t;

        Transformation temp;
        auto const& transformation = volume.transformation_at(ray.time, temp);

        uint32_t const max_samples = static_cast<uint32_t>(std::ceil(range / settings_.step_size));
        uint32_t const num_samples = ray.is_primary() ? max_samples : 1;

        float const step = range / static_cast<float>(num_samples);

        float3 radiance(0.f);
        float3 tr(1.f);

        float3 const start = ray.point(min_t);

        float const r = std::max(rng_.random_float(), 0.0001f);

        min_t += r * step;

        float3 const next = ray.point(min_t);
        Ray tau_ray(start, next - start, 0.f, 1.f, 0, ray.time);

        float3 const tau_ray_direction     = ray.point(min_t + step) - next;
        float3 const inv_tau_ray_direction = math::reciprocal(tau_ray_direction);

        auto const& material = *volume.material(0);

        for (uint32_t i = num_samples; i > 0; --i, min_t += step) {
                // This happens sometimes when the range is very small compared to the world
coordinates. if (float3::identity() == tau_ray.direction) { tau_ray.origin = ray.point(min_t +
step); tau_ray.direction = tau_ray_direction; tau_ray.inv_direction = inv_tau_ray_direction;
                        continue;
                }

                float3 const tau = material.optical_depth(transformation, volume.aabb(), tau_ray,
                                                                                                  settings_.step_size,
rng_, Sampler_filter::Undefined, worker);

                tr *= math::exp(-tau);

                float3 const current = ray.point(min_t);
                tau_ray.origin = current;
                // This stays the same during the loop,
                // but we need a different value in the first iteration.
                // Would be nicer to restructure the loop.
                tau_ray.direction = tau_ray_direction;
                tau_ray.inv_direction = inv_tau_ray_direction;

                auto& material_sample = sample(-ray.direction, ray.time, material,
                                                                           Sampler_filter::Undefined,
worker);

                float3 local_radiance(0.f);

                // Lighting
                uint32_t const num_light_samples = 1;
                for (uint32_t j = num_light_samples; j > 0; --j) {
                        auto const light = worker.scene().random_light(rng_.random_float());

                        scene::light::Sample light_sample;
                        if (light.ref.sample(start, float3::identity(), ray.time, true, sampler_, 0,
                                                                 Sampler_filter::Nearest, worker,
light_sample)) {

                                auto const bxdf = material_sample.evaluate(light_sample.shape.wi);

                                local_radiance += (light_sample.radiance * bxdf.reflection)
                                                                / (light.pdf *
light_sample.shape.pdf);
                        }
                }

                float3 mu_a, scattering;
                material.collision_coefficients(transformation, current,
                                                        Sampler_filter::Undefined, worker, mu_a,
scattering);

                radiance += tr * scattering * local_radiance;
        }

        transmittance = tr;

        float3 const color = step * radiance;

        return color;
}
*/

const scene::material::Sample& Aerial_perspective::sample(f_float3 wo, float time,
                                                          const Material& material,
                                                          Sampler_filter filter, Worker& worker) {
    scene::Renderstate rs;

    rs.time = time;

    return material.sample(wo, rs, filter, sampler_, worker);
}

Aerial_perspective_factory::Aerial_perspective_factory(take::Settings const& take_settings,
                                                       uint32_t num_integrators, float step_size,
                                                       bool shadows)
    : Factory(take_settings, num_integrators),
      integrators_(memory::allocate_aligned<Aerial_perspective>(num_integrators)),
      settings_{step_size, !shadows} {}

Aerial_perspective_factory::~Aerial_perspective_factory() {
    memory::free_aligned(integrators_);
}

Integrator* Aerial_perspective_factory::create(uint32_t id, rnd::Generator& rng) const {
    return new (&integrators_[id]) Aerial_perspective(rng, take_settings_, settings_);
}

}  // namespace rendering::integrator::volume
