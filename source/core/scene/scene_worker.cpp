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

using Texture_sampler_2D = image::texture::sampler::Sampler_2D;
using Texture_sampler_3D = image::texture::sampler::Sampler_3D;

Worker::Worker() noexcept : node_stack_(128 + 16) {}

void Worker::init(uint32_t id, take::Settings const& settings, Scene const& scene,
                  uint32_t max_material_sample_size, uint32_t max_material_sample_depth) noexcept {
    id_       = id;
    rng_      = rnd::Generator(0, id);
    settings_ = settings;
    scene_    = &scene;
    sample_cache_.init(max_material_sample_size, max_material_sample_depth);
}

Worker::~Worker() noexcept {}

uint32_t Worker::id() const noexcept {
    return id_;
}

bool Worker::intersect(Ray& ray, Intersection& intersection) const noexcept {
    return scene_->intersect(ray, node_stack_, intersection);
}

bool Worker::intersect(Ray& ray, float& epsilon) const noexcept {
    return scene_->intersect(ray, node_stack_, epsilon);
}

bool Worker::resolve_mask(Ray& ray, Intersection& intersection, Sampler_filter filter) noexcept {
    float const ray_offset_factor = settings_.ray_offset_factor;

    float const start_min_t = ray.min_t;

    float opacity = intersection.opacity(ray.time, filter, *this);

    while (opacity < 1.f) {
        if (opacity > 0.f && opacity > rng_.random_float()) {
            ray.min_t = start_min_t;
            return true;
        }

        // Slide along ray until opaque surface is found
        ray.min_t = ray.max_t + ray_offset_factor * intersection.geo.epsilon;
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

bool Worker::intersect_and_resolve_mask(Ray& ray, Intersection& intersection,
                                        Sampler_filter filter) noexcept {
    if (!intersect(ray, intersection)) {
        return false;
    }

    return resolve_mask(ray, intersection, filter);
}

bool Worker::visibility(Ray const& ray) const noexcept {
    return !scene_->intersect_p(ray, node_stack_);
}

bool Worker::masked_visibility(Ray const& ray, Sampler_filter filter, float& mv) const noexcept {
    float o;
    if (bool const visible = scene_->opacity(ray, filter, *this, o); visible) {
        mv = 1.f - o;
        return true;
    }

    return false;
}

Scene const& Worker::scene() const noexcept {
    return *scene_;
}

shape::Node_stack& Worker::node_stack() const noexcept {
    return node_stack_;
}

material::Sample_cache& Worker::sample_cache() const noexcept {
    return sample_cache_;
}

Texture_sampler_2D const& Worker::sampler_2D(uint32_t key, Sampler_filter filter) const noexcept {
    return sampler_cache_.sampler_2D(key, filter);
}

Texture_sampler_3D const& Worker::sampler_3D(uint32_t key, Sampler_filter filter) const noexcept {
    return sampler_cache_.sampler_3D(key, filter);
}

prop::Interface_stack& Worker::interface_stack() noexcept {
    return interface_stack_;
}

float Worker::ior_outside(float3 const& wo, Intersection const& intersection) const noexcept {
    if (intersection.same_hemisphere(wo)) {
        return interface_stack_.top_ior();
    } else {
        return interface_stack_.peek_ior(intersection);
    }
}

void Worker::interface_change(float3 const& dir, Intersection const& intersection) noexcept {
    if (intersection.same_hemisphere(dir)) {
        interface_stack_.remove(intersection);
    } else if (interface_stack_.top_is_vacuum() || intersection.material()->ior() > 1.f) {
        interface_stack_.push(intersection);
    }
}

}  // namespace scene
