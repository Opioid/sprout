#include "rendering_worker.hpp"
#include "base/math/sampling/sample_distribution.hpp"
#include "base/math/vector4.inl"
#include "base/memory/align.hpp"
#include "base/spectrum/rgb.hpp"
#include "rendering/integrator/surface/surface_integrator.hpp"
#include "rendering/integrator/volume/volume_integrator.hpp"
#include "sampler/sampler.hpp"
#include "scene/material/material.hpp"
#include "scene/prop/interface_stack.inl"
#include "scene/prop/prop.hpp"
#include "scene/prop/prop_intersection.inl"
#include "scene/scene.hpp"
#include "scene/scene_constants.hpp"
#include "scene/scene_ray.inl"

#include "base/debug/assert.hpp"

namespace rendering {

Worker::~Worker() {
    memory::safe_destruct(sampler_);
    memory::safe_destruct(volume_integrator_);
    memory::safe_destruct(surface_integrator_);
}

void Worker::init(uint32_t id, take::Settings const& settings, scene::Scene const& scene,
                  uint32_t                      max_sample_size,
                  integrator::surface::Factory& surface_integrator_factory,
                  integrator::volume::Factory&  volume_integrator_factory,
                  sampler::Factory&             sampler_factory) {
    scene::Worker::init(id, settings, scene, max_sample_size);

    surface_integrator_ = surface_integrator_factory.create(id, rng_);
    volume_integrator_  = volume_integrator_factory.create(id, rng_);
    sampler_            = sampler_factory.create(id, rng_);
}

void Worker::prepare(uint32_t num_samples_per_pixel) {
    surface_integrator_->prepare(*scene_, num_samples_per_pixel);
    volume_integrator_->prepare(*scene_, num_samples_per_pixel);
    sampler_->resize(num_samples_per_pixel, 1, 2, 1);
}

float4 Worker::li(Ray& ray, const scene::prop::Interface_stack& interface_stack) {
    interface_stack_ = interface_stack;

    scene::prop::Intersection intersection;

    if (!interface_stack_.empty()) {
        float3 vli, vtr;
        if (!volume_integrator_->integrate(ray, intersection, Sampler_filter::Undefined, *this, vli,
                                           vtr)) {
            return float4(vli, spectrum::luminance(vli));
        }

        float3 const li = surface_integrator_->li(ray, intersection, *this);

        SOFT_ASSERT(math::all_finite_and_positive(li));

        return float4(vtr * li + vli, 1.f);
    } else if (intersect_and_resolve_mask(ray, intersection, Sampler_filter::Undefined)) {
        float3 const li = surface_integrator_->li(ray, intersection, *this);

        SOFT_ASSERT(math::all_finite_and_positive(li));

        return float4(li, 1.f);
    } else {
        return float4(0.f);
    }
}

bool Worker::volume(Ray& ray, Intersection& intersection, Sampler_filter filter, float3& li,
                    float3& transmittance) {
    return volume_integrator_->integrate(ray, intersection, filter, *this, li, transmittance);
}

float3 Worker::transmittance(Ray const& ray) {
    float3 transmittance(1.f);

    if (!scene_->has_volumes()) {
        return transmittance;
    }

    interface_stack_temp_ = interface_stack_;

    // This is the typical SSS case:
    // A medium is on the stack but we already considered it during shadow calculation,
    // igonoring the IoR. Therefore remove the medium from the stack.
    if (interface_stack_.top_ior() > 1.f) {
        interface_stack_.pop();
    }

    float const ray_max_t = ray.max_t;

    Ray tray = ray;

    Intersection intersection;

    for (;;) {
        bool const hit = scene_->intersect_volume(tray, node_stack_, intersection);

        SOFT_ASSERT(tray.max_t > tray.min_t);

        if (!interface_stack_.empty()) {
            float3 const tr = volume_integrator_->transmittance(tray, *this);
            transmittance *= math::saturate(tr);
        }

        if (!hit) {
            break;
        }

        if (intersection.same_hemisphere(ray.direction)) {
            interface_stack_.remove(intersection);
        } else {
            interface_stack_.push(intersection);
        }

        tray.min_t = tray.max_t + intersection.geo.epsilon * settings_.ray_offset_factor;
        tray.max_t = ray_max_t;
    }

    interface_stack_.swap(interface_stack_temp_);

    return transmittance;
}

float3 Worker::tinted_visibility(Ray const& ray, Sampler_filter filter) {
    return float3(1.f) - scene_->thin_absorption(ray, filter, *this);
}

float3 Worker::tinted_visibility(Ray& ray, Intersection const& intersection,
                                 Sampler_filter filter) {
    if (intersection.geo.subsurface && intersection.material()->ior() > 1.f) {
        float const ray_max_t = ray.max_t;

        float epsilon;
        if (intersect(ray, epsilon)) {
            float3 const tr = volume_integrator_->transmittance(ray, *this);

            SOFT_ASSERT(math::all_finite_and_positive(tr));

            ray.min_t = ray.max_t + epsilon * settings_.ray_offset_factor;
            ray.max_t = ray_max_t;

            return tr * tinted_visibility(ray, filter);
        }
    }

    return tinted_visibility(ray, filter);
}

sampler::Sampler* Worker::sampler() {
    return sampler_;
}

scene::prop::Interface_stack& Worker::interface_stack() {
    return interface_stack_;
}

void Worker::interface_change(f_float3 dir, Intersection const& intersection) {
    if (intersection.same_hemisphere(dir)) {
        interface_stack_.remove(intersection);
    } else if (interface_stack_.top_ior() == 1.f || intersection.material()->ior() > 1.f) {
        interface_stack_.push(intersection);
    }
}

}  // namespace rendering
