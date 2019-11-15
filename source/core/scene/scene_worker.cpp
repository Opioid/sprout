#include "scene_worker.hpp"
#include "base/math/matrix4x4.inl"
#include "base/math/vector4.inl"
#include "base/random/generator.inl"
#include "material/material_sample.hpp"
#include "material/sampler_cache.hpp"
#include "prop/interface_stack.inl"
#include "prop/prop.hpp"
#include "prop/prop_intersection.inl"
#include "scene.hpp"
#include "scene_constants.hpp"
#include "scene_ray.inl"
#include "shape/node_stack.inl"

namespace scene {

static material::Sampler_cache const Sampler_cache;

using Texture_sampler_2D = image::texture::Sampler_2D;
using Texture_sampler_3D = image::texture::Sampler_3D;

Worker::Worker() noexcept : node_stack_(128 + 16) {}

Worker::~Worker() noexcept {}

void Worker::init(uint32_t id, Scene const& scene, Camera const& camera,
                  uint32_t max_sample_size) noexcept {
    rng_.start(0, id);
    scene_  = &scene;
    camera_ = &camera;
    sample_cache_.init(max_sample_size);
}

bool Worker::resolve_mask(Ray& ray, Intersection& intersection, Filter filter) noexcept {
    float const start_min_t = ray.min_t;

    float opacity = intersection.opacity(ray.time, filter, *this);

    while (opacity < 1.f) {
        if (opacity > 0.f && opacity > rng_.random_float()) {
            ray.min_t = start_min_t;
            return true;
        }

        // Slide along ray until opaque surface is found
        ray.min_t = offset_f(ray.max_t);
        ray.max_t = scene::Ray_max_t;
        if (!intersect(ray, intersection)) {
            ray.min_t = start_min_t;
            return false;
        }

        opacity = intersection.opacity(ray.time, filter, *this);
    }

    ray.min_t = start_min_t;
    return true;
}

Texture_sampler_2D const& Worker::sampler_2D(uint32_t key, Filter filter) const noexcept {
    return Sampler_cache.sampler_2D(key, filter);
}

Texture_sampler_3D const& Worker::sampler_3D(uint32_t key, Filter filter) const noexcept {
    return Sampler_cache.sampler_3D(key, filter);
}

void Worker::reset_interface_stack(Interface_stack const& stack) noexcept {
    interface_stack_ = stack;
}

float Worker::ior_outside(float3 const& wo, Intersection const& intersection) const noexcept {
    if (intersection.same_hemisphere(wo)) {
        return interface_stack_.top_ior(*this);
    } else {
        return interface_stack_.peek_ior(intersection, *this);
    }
}

void Worker::interface_change(float3 const& dir, Intersection const& intersection) noexcept {
    if (intersection.same_hemisphere(dir)) {
        interface_stack_.remove(intersection);
    } else if (interface_stack_.top_is_vacuum(*this) |
               (intersection.material(*this)->ior() > 1.f)) {
        interface_stack_.push(intersection);
    }
}

material::IoR Worker::interface_change_ior(float3 const&       dir,
                                           Intersection const& intersection) noexcept {
    bool const leave = intersection.same_hemisphere(dir);

    material::IoR ior;

    if (leave) {
        ior.eta_t = interface_stack_.peek_ior(intersection, *this);
        ior.eta_i = intersection.material(*this)->ior();

        interface_stack_.remove(intersection);
    } else {
        ior.eta_t = intersection.material(*this)->ior();
        ior.eta_i = interface_stack_.top_ior(*this);

        if (interface_stack_.top_is_vacuum(*this) | (intersection.material(*this)->ior() > 1.f)) {
            interface_stack_.push(intersection);
        }
    }

    return ior;
}

}  // namespace scene
