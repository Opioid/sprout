#ifndef SU_CORE_SCENE_WORKER_INL
#define SU_CORE_SCENE_WORKER_INL

#include "camera/camera.hpp"
#include "material/material_sample_cache.inl"
#include "scene/scene.inl"
#include "scene_worker.hpp"

namespace scene {

inline bool Worker::intersect(Ray& ray, Intersection& intersection) const noexcept {
    return scene_->intersect(ray, *this, intersection);
}

inline bool Worker::intersect(Ray& ray, shape::Normals& normals) const noexcept {
    return scene_->intersect(ray, *this, normals);
}

inline bool Worker::intersect_and_resolve_mask(Ray& ray, Intersection& intersection,
                                               Filter filter) noexcept {
    if (!intersect(ray, intersection)) {
        return false;
    }

    return resolve_mask(ray, intersection, filter);
}

inline bool Worker::visibility(Ray const& ray) const noexcept {
    return !scene_->intersect_p(ray, *this);
}

inline bool Worker::masked_visibility(Ray const& ray, Filter filter, float& mv) const noexcept {
    return scene_->visibility(ray, filter, *this, mv);
}

inline Scene const& Worker::scene() const noexcept {
    return *scene_;
}

inline camera::Camera const& Worker::camera() const noexcept {
    return *camera_;
}

inline uint64_t Worker::absolute_time(uint32_t frame, float frame_delta) const noexcept {
    return camera_->absolute_time(frame, frame_delta);
}

inline shape::Node_stack& Worker::node_stack() const noexcept {
    return node_stack_;
}

inline material::Sample_cache& Worker::sample_cache() const noexcept {
    return sample_cache_;
}

inline image::texture::Texture const* Worker::texture(uint32_t id) const noexcept {
    return scene_->texture(id);
}

inline prop::Interface_stack& Worker::interface_stack() noexcept {
    return interface_stack_;
}

template <typename T>
T& Worker::sample() const noexcept {
    return sample_cache_.get<T>();
}

}  // namespace scene

#endif
