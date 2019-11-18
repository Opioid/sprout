#include "rendering_worker.hpp"
#include "base/math/sample_distribution.inl"
#include "base/math/vector4.inl"
#include "base/memory/align.hpp"
#include "base/spectrum/rgb.hpp"
#include "rendering/integrator/integrator_helper.hpp"
#include "rendering/integrator/particle/lighttracer.hpp"
#include "rendering/integrator/particle/photon/photon_map.hpp"
#include "rendering/integrator/particle/photon/photon_mapper.hpp"
#include "rendering/integrator/surface/surface_integrator.hpp"
#include "rendering/integrator/volume/volume_integrator.hpp"
#include "sampler/sampler.hpp"
#include "scene/material/material.hpp"
#include "scene/prop/interface_stack.inl"
#include "scene/prop/prop.hpp"
#include "scene/prop/prop_intersection.inl"
#include "scene/scene.inl"
#include "scene/scene_constants.hpp"
#include "scene/scene_ray.inl"
#include "scene/scene_worker.inl"
#include "take/take_view.hpp"

#include "base/debug/assert.hpp"

namespace rendering {

Worker::~Worker() noexcept {
    delete photon_mapper_;
    memory::destroy(sampler_);
    memory::destroy(volume_integrator_);
    memory::destroy(surface_integrator_);
}

void Worker::init(uint32_t id, Scene const& scene, Camera const& camera, uint32_t max_sample_size,
                  uint32_t num_samples_per_pixel, Surface_factory& surface_factory,
                  Volume_factory& volume_factory, sampler::Factory& sampler_factory,
                  Photon_map* photon_map, take::Photon_settings const& photon_settings,
                  Lighttracer_factory* lighttracer_factory, uint32_t num_particles_per_chunk,
                  Particle_importance* particle_importance) noexcept {
    scene::Worker::init(id, scene, camera, max_sample_size);

    surface_integrator_ = surface_factory.create(id, rng_);
    surface_integrator_->prepare(scene, num_samples_per_pixel);

    volume_integrator_ = volume_factory.create(id, rng_);
    volume_integrator_->prepare(scene, num_samples_per_pixel);

    sampler_ = sampler_factory.create(id, rng_);
    sampler_->resize(num_samples_per_pixel, 1, 2, 1);

    if (photon_settings.num_photons) {
        Photon_mapper::Settings const ps{photon_settings.max_bounces,
                                         photon_settings.indirect_photons,
                                         photon_settings.full_light_path};

        photon_mapper_ = new Photon_mapper(rng_, ps);
        photon_mapper_->prepare(scene, 0);

        photon_map_ = photon_map;
    }

    if (lighttracer_factory) {
        lighttracer_ = lighttracer_factory->create(id, rng_);
        lighttracer_->prepare(scene, num_particles_per_chunk);
    }

    particle_importance_ = particle_importance;
}

float4 Worker::li(Ray& ray, Interface_stack const& interface_stack) noexcept {
    Intersection intersection;

    if (!interface_stack.empty()) {
        reset_interface_stack(interface_stack);

        float3 vli, vtr;
        if (auto const event = volume_integrator_->integrate(ray, intersection, Filter::Undefined,
                                                             *this, vli, vtr);
            Event::Absorb == event) {
            return float4(vli, 1.f);
        }

        Interface_stack temp_stack = interface_stack_;

        float4 const li = surface_integrator_->li(ray, intersection, *this, temp_stack);

        SOFT_ASSERT(all_finite_and_positive(li));

        return float4(vtr * li.xyz() + vli, li[3]);
    } else if (intersect_and_resolve_mask(ray, intersection, Filter::Undefined)) {
        float4 const li = surface_integrator_->li(ray, intersection, *this, interface_stack);

        SOFT_ASSERT(all_finite_and_positive(li));

        return li;
    } else {
        return float4(0.f);
    }
}

bool Worker::transmitted(Ray& ray, float3 const& wo, Intersection const& intersection,
                         Filter filter, float3& tr) noexcept {
    if (float3 a; tinted_visibility(ray, wo, intersection, filter, a)) {
        if (float3 b; transmittance(ray, b)) {
            tr = a * b;
            return true;
        }
    }

    return false;
}

uint32_t Worker::bake_photons(int32_t begin, int32_t end, uint32_t frame,
                              uint32_t iteration) noexcept {
    if (photon_mapper_) {
        return photon_mapper_->bake(*photon_map_, begin, end, frame, iteration, *this);
    }

    return 0;
}

float3 Worker::photon_li(Intersection const& intersection, Material_sample const& sample) const
    noexcept {
    if (photon_map_) {
        return photon_map_->li(intersection, sample, *this);
    }

    return float3(0.f);
}

Worker::Particle_importance& Worker::particle_importance() const noexcept {
    return *particle_importance_;
}

bool Worker::transmittance(Ray const& ray, float3& transmittance) noexcept {
    if (!scene_->has_volumes()) {
        transmittance = float3(1.f);
        return true;
    }

    interface_stack_temp_ = interface_stack_;

    // This is the typical SSS case:
    // A medium is on the stack but we already considered it during shadow calculation,
    // igonoring the IoR. Therefore remove the medium from the stack.
    if (!interface_stack_.top_is_vacuum(*this)) {
        interface_stack_.pop();
    }

    float const ray_max_t = ray.max_t;

    Ray tray = ray;

    Intersection intersection;

    float3 w(1.f);

    for (;;) {
        bool const hit = scene_->intersect_volume(tray, *this, intersection);

        SOFT_ASSERT(tray.max_t > tray.min_t);

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

        if (intersection.same_hemisphere(tray.direction)) {
            interface_stack_.remove(intersection);
        } else {
            interface_stack_.push(intersection);
        }

        tray.min_t = scene::offset_f(tray.max_t);
        tray.max_t = ray_max_t;

        if (tray.min_t > tray.max_t) {
            break;
        }
    }

    interface_stack_.swap(interface_stack_temp_);

    transmittance = w;
    return true;
}

bool Worker::tinted_visibility(Ray& ray, float3 const& wo, Intersection const& intersection,
                               Filter filter, float3& tv) noexcept {
    auto const& material = *intersection.material(*this);

    if (intersection.subsurface & (material.ior() > 1.f)) {
        float const ray_max_t = ray.max_t;

        if (scene::shape::Normals normals; intersect(ray, normals)) {
            if (float3 tr; volume_integrator_->transmittance(ray, *this, tr)) {
                SOFT_ASSERT(all_finite_and_positive(tr));

                ray.min_t = scene::offset_f(ray.max_t);
                ray.max_t = ray_max_t;

                if (scene_->thin_absorption(ray, filter, *this, tv)) {
                    float3 const wi = ray.direction;

                    float const vbh = material.volume_border_hack(wi, normals.n, *this);

                    float const nsc = non_symmetry_compensation(wi, wo, normals.geo_n, normals.n);

                    tv *= vbh * nsc * tr;

                    return true;
                }
            }

            return false;
        }
    }

    return scene_->thin_absorption(ray, filter, *this, tv);
}

}  // namespace rendering
