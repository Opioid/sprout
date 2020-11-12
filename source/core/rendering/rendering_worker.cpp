#include "rendering_worker.hpp"
#include "base/math/sample_distribution.inl"
#include "base/math/vector4.inl"
#include "base/memory/align.hpp"
#include "base/random/generator.inl"
#include "base/spectrum/rgb.hpp"
#include "rendering/integrator/integrator_helper.hpp"
#include "rendering/integrator/particle/lighttracer.hpp"
#include "rendering/integrator/particle/photon/photon_map.hpp"
#include "rendering/integrator/particle/photon/photon_mapper.hpp"
#include "rendering/integrator/surface/surface_integrator.hpp"
#include "rendering/integrator/volume/volume_integrator.hpp"
#include "rendering/sensor/aov/value.hpp"
#include "rendering/sensor/sensor.hpp"
#include "sampler/camera_sample.hpp"
#include "sampler/sampler.hpp"
#include "scene/material/material.inl"
#include "scene/material/material_helper.hpp"
#include "scene/prop/interface_stack.inl"
#include "scene/prop/prop.hpp"
#include "scene/prop/prop_intersection.inl"
#include "scene/scene.inl"
#include "scene/scene_constants.hpp"
#include "scene/scene_ray.inl"
#include "scene/scene_worker.inl"
#include "take/take.hpp"

#include "base/debug/assert.hpp"

#include <iostream>

namespace rendering {

Worker::Worker() = default;

Worker::~Worker() {
    delete photon_mapper_;
}

void Worker::init(uint32_t id, Scene const& scene, Camera const& camera,
                  uint32_t num_samples_per_pixel, Surface_pool const* surfaces,
                  Volume_pool const& volumes, sampler::Pool const& samplers, Photon_map* photon_map,
                  take::Photon_settings const& photon_settings,
                  Lighttracer_pool const* lighttracers, uint32_t num_particles_per_chunk,
                  AOV_pool const& aovs, Particle_importance* particle_importance) {
    scene::Worker::init(scene, camera);

    if (surfaces) {
        surface_integrator_ = surfaces->get(id);
        surface_integrator_->prepare(scene, num_samples_per_pixel);
    }

    volume_integrator_ = volumes.get(id);
    volume_integrator_->prepare(scene, num_samples_per_pixel);

    sampler_ = samplers.get(id);
    sampler_->resize(num_samples_per_pixel, 1, 2, 1);

    if (photon_settings.num_photons > 0) {
        delete photon_mapper_;

        Photon_mapper::Settings const ps{photon_settings.max_bounces,
                                         photon_settings.full_light_path};

        photon_mapper_ = new Photon_mapper(ps);
        photon_mapper_->prepare(scene, 0);
    }

    photon_map_ = photon_map;

    if (lighttracers) {
        lighttracer_ = lighttracers->get(id);
        lighttracer_->prepare(scene, num_particles_per_chunk);
    }

    aov_ = aovs.get(id);

    particle_importance_ = particle_importance;
}

void Worker::render(uint32_t frame, uint32_t view, uint32_t iteration, int4 const& tile,
                    uint32_t num_samples) {
    Camera const& camera = *camera_;

    int2 const offset = camera.view_offset(view);

    int4 crop = camera.crop();
    crop[2] -= crop[0] + 1;
    crop[3] -= crop[1] + 1;
    crop[0] += offset[0];
    crop[1] += offset[1];

    int4 const view_tile(offset + tile.xy(), offset + tile.zw());

    auto& sensor = camera.sensor();

    int4 isolated_bounds = sensor.isolated_tile(view_tile);
    isolated_bounds[2] -= isolated_bounds[0];
    isolated_bounds[3] -= isolated_bounds[1];

    int32_t const fr = sensor.filter_radius_int();

    int2 const r = camera.resolution() + 2 * fr;

    uint64_t const o0 = uint64_t(iteration) * uint64_t(r[0] * r[1]);

    AOV* aov = aov_;

    for (int32_t y = tile[1], y_back = tile[3]; y <= y_back; ++y) {
        uint64_t const o1 = uint64_t((y + fr) * r[0]) + o0;
        for (int32_t x = tile[0], x_back = tile[2]; x <= x_back; ++x) {
            rng_.start(0, o1 + uint64_t(x + fr));

            sampler_->start_pixel(rng(), num_samples);
            surface_integrator_->start_pixel(rng(), num_samples);
            volume_integrator_->start_pixel(rng(), num_samples);

            int2 const pixel(x, y);

            for (uint32_t i = num_samples; i > 0; --i) {
                if (aov) {
                    aov->clear();
                }

                auto const sample = sampler_->camera_sample(rng(), pixel);

                if (Ray ray; camera.generate_ray(sample, frame, view, *scene_, ray)) {
                    float4 const color = li(ray, camera.interface_stack(), aov);
                    sensor.add_sample(sample, color, aov, isolated_bounds, offset, crop);
                } else {
                    sensor.add_sample(sample, float4(0.f), aov, isolated_bounds, offset, crop);
                }
            }
        }
    }
}

void Worker::render_track_variance(uint32_t frame, uint32_t view, int4 const& tile,
                                   uint32_t num_samples) {
    Camera const& camera = *camera_;

    int2 const offset = camera.view_offset(view);

    int4 crop = camera.crop();
    crop[2] -= crop[0] + 1;
    crop[3] -= crop[1] + 1;
    crop[0] += offset[0];
    crop[1] += offset[1];

    int4 const view_tile(offset + tile.xy(), offset + tile.zw());

    auto& sensor = camera.sensor();

    int4 isolated_bounds = sensor.isolated_tile(view_tile);
    isolated_bounds[2] -= isolated_bounds[0];
    isolated_bounds[3] -= isolated_bounds[1];

    int32_t const fr = sensor.filter_radius_int();

    int2 const r = camera.resolution() + 2 * fr;

    AOV* aov = aov_;

    for (int32_t y = tile[1], y_back = tile[3]; y <= y_back; ++y) {
        uint64_t const o1 = uint64_t((y + fr) * r[0]);
        for (int32_t x = tile[0], x_back = tile[2]; x <= x_back; ++x) {
            rng_.start(0, o1 + uint64_t(x + fr));

            sampler_->start_pixel(rng(), num_samples);
            surface_integrator_->start_pixel(rng(), num_samples);
            volume_integrator_->start_pixel(rng(), num_samples);

            int2 const pixel(x, y);

            float old_m;
            float new_m;
            float old_s;
            float new_s;
            float total = 0.f;

            for (uint32_t i = num_samples, n = 1; i > 0; --i, ++n) {
                if (aov) {
                    aov->clear();
                }

                auto const sample = sampler_->camera_sample(rng(), pixel);

                float value = 0.f;

                if (Ray ray; camera.generate_ray(sample, frame, view, *scene_, ray)) {
                    float4 const color   = li(ray, camera.interface_stack(), aov);
                    float4 const clamped = sensor.add_sample(sample, color, aov, isolated_bounds,
                                                             offset, crop);

                    value = max_component(clamped.xyz());
                } else {
                    sensor.add_sample(sample, float4(0.f), aov, isolated_bounds, offset, crop);
                }

                // https://www.johndcook.com/blog/standard_deviation/
                if (1 == n) {
                    old_m = new_m = value;
                    old_s         = 0.0;
                } else {
                    new_m = old_m + (value - old_m) / n;
                    new_s = old_s + (value - old_m) * (value - new_m);

                    // set up for next iteration
                    old_m = new_m;
                    old_s = new_s;
                }

                total += value;
            }

            float const average  = total / float(num_samples);
            float const variance = new_s / float(num_samples - 1);

            sensor.set_variance(
                pixel, average > 0.f ? std::sqrt(variance) / std::max(average, 0.02f) : 0.f);
        }
    }
}

void Worker::render_use_variance(uint32_t frame, uint32_t view, int4 const& tile,
                                 uint32_t max_samples) {
    Camera const& camera = *camera_;

    int2 const offset = camera.view_offset(view);

    int4 crop = camera.crop();
    crop[2] -= crop[0] + 1;
    crop[3] -= crop[1] + 1;
    crop[0] += offset[0];
    crop[1] += offset[1];

    int4 const view_tile(offset + tile.xy(), offset + tile.zw());

    auto& sensor = camera.sensor();

    int4 isolated_bounds = sensor.isolated_tile(view_tile);
    isolated_bounds[2] -= isolated_bounds[0];
    isolated_bounds[3] -= isolated_bounds[1];

    int32_t const fr = sensor.filter_radius_int();

    int2 const r = camera.resolution() + 2 * fr;

    uint64_t const o0 = uint64_t(r[0] * r[1]);

    AOV* aov = aov_;

    for (int32_t y = tile[1], y_back = tile[3]; y <= y_back; ++y) {
        uint64_t const o1 = uint64_t((y + fr) * r[0]) + o0;
        for (int32_t x = tile[0], x_back = tile[2]; x <= x_back; ++x) {
            int2 const pixel(x, y);

            uint32_t const num_samples = sensor.num_samples_by_estimate(pixel, max_samples);

            rng_.start(0, o1 + uint64_t(x + fr));

            sampler_->start_pixel(rng(), num_samples);
            surface_integrator_->start_pixel(rng(), num_samples);
            volume_integrator_->start_pixel(rng(), num_samples);

            for (uint32_t i = num_samples; i > 0; --i) {
                auto const sample = sampler_->camera_sample(rng(), pixel);

                if (Ray ray; camera.generate_ray(sample, frame, view, *scene_, ray)) {
                    float4 const color = li(ray, camera.interface_stack(), aov);
                    sensor.add_sample(sample, color, aov, isolated_bounds, offset, crop);
                } else {
                    sensor.add_sample(sample, float4(0.f), aov, isolated_bounds, offset, crop);
                }
            }
        }
    }
}

void Worker::particles(uint32_t frame, uint64_t offset, ulong2 const& range) {
    Camera const& camera = *camera_;

    uint32_t const num_samples = uint32_t(range[1] - range[0]);

    lighttracer_->start_pixel(rng(), num_samples);

    auto const& interface_stack = camera.interface_stack();

    for (uint64_t i = range[0]; i < range[1]; ++i) {
        rng_.start(0, i + offset);

        lighttracer_->li(frame, *this, interface_stack);
    }
}

bool Worker::transmitted(Ray& ray, float3 const& wo, Intersection const& isec, Filter filter,
                         float3& tr) {
    if (float3 a; tinted_visibility(ray, wo, isec, filter, a)) {
        if (float3 b; transmittance(ray, b)) {
            tr = a * b;
            return true;
        }
    }

    return false;
}

uint32_t Worker::bake_photons(int32_t begin, int32_t end, uint32_t frame, uint32_t iteration) {
    if (photon_mapper_) {
        return photon_mapper_->bake(*photon_map_, begin, end, frame, iteration, *this);
    }

    return 0;
}

float3 Worker::photon_li(Intersection const& isec, Material_sample const& sample) const {
    if (photon_map_) {
        return photon_map_->li(isec, sample, *this);
    }

    return float3(0.f);
}

Worker::Particle_importance& Worker::particle_importance() const {
    return *particle_importance_;
}

float4 Worker::li(Ray& ray, Interface_stack const& interface_stack, AOV* aov) {
    Intersection isec;

    if (!interface_stack.empty()) {
        reset_interface_stack(interface_stack);

        float3 vli;
        float3 vtr;
        if (auto const event = volume_integrator_->integrate(ray, isec, Filter::Undefined, *this,
                                                             vli, vtr);
            Event::Absorb == event) {
            return float4(vli, 1.f);
        }

        Interface_stack const temp_stack = interface_stack_;

        float4 const li = surface_integrator_->li(ray, isec, *this, temp_stack, aov);

        SOFT_ASSERT(all_finite_and_positive(li));

        return float4(vtr * li.xyz() + vli, li[3]);
    }

    if (intersect_and_resolve_mask(ray, isec, Filter::Undefined)) {
        float4 const li = surface_integrator_->li(ray, isec, *this, interface_stack, aov);

        SOFT_ASSERT(all_finite_and_positive(li));

        return li;
    }

    return float4(0.f);
}

bool Worker::transmittance(Ray const& ray, float3& transmittance) {
    if (!scene_->has_volumes()) {
        transmittance = float3(1.f);
        return true;
    }

    interface_stack_temp_ = interface_stack_;

    // This is the typical SSS case:
    // A medium is on the stack but we already considered it during shadow calculation,
    // ignoring the IoR. Therefore remove the medium from the stack.
    if (!interface_stack_.straight(*this)) {
        interface_stack_.pop();
    }

    float const ray_max_t = ray.max_t();

    Ray tray = ray;

    Intersection isec;

    float3 w(1.f);

    for (;;) {
        bool const hit = scene_->intersect_volume(tray, *this, isec);

        SOFT_ASSERT(tray.max_t() >= tray.min_t());

        if (!interface_stack_.empty()) {
            if (float3 tr; volume_integrator_->transmittance(tray, *this, tr)) {
                w *= saturate(tr);
            } else {
                return false;
            }
        }

        if (!hit) {
            break;
        }

        if (isec.same_hemisphere(tray.direction)) {
            interface_stack_.remove(isec);
        } else {
            interface_stack_.push(isec);
        }

        tray.min_t() = scene::offset_f(tray.max_t());
        tray.max_t() = ray_max_t;

        if (tray.min_t() > tray.max_t()) {
            break;
        }
    }

    interface_stack_.swap(interface_stack_temp_);

    transmittance = w;
    return true;
}

bool Worker::tinted_visibility(Ray& ray, float3 const& wo, Intersection const& isec, Filter filter,
                               float3& tv) {
    using namespace scene::material;

    auto const& material = *isec.material(*this);

    if (isec.subsurface & (material.ior() > 1.f)) {
        float const ray_max_t = ray.max_t();

        if (scene::shape::Normals normals; intersect(ray, normals)) {
            if (float3 tr; volume_integrator_->transmittance(ray, *this, tr)) {
                SOFT_ASSERT(all_finite_and_positive(tr));

                ray.min_t() = scene::offset_f(ray.max_t());
                ray.max_t() = ray_max_t;

                if (scene_->tinted_visibility(ray, filter, *this, tv)) {
                    float3 const wi = ray.direction;

                    float const vbh = material.border(wi, normals.n);
                    float const nsc = non_symmetry_compensation(wi, wo, normals.geo_n, normals.n);

                    tv *= vbh * nsc * tr;

                    return true;
                }
            }

            return false;
        }
    }

    return scene_->tinted_visibility(ray, filter, *this, tv);
}

}  // namespace rendering
