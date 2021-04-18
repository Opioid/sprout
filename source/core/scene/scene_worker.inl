#ifndef SU_CORE_SCENE_WORKER_INL
#define SU_CORE_SCENE_WORKER_INL

#include "camera/camera.hpp"
#include "material/material_sample_cache.inl"
#include "scene/scene.inl"
#include "scene_worker.hpp"

namespace scene {

inline bool Worker::intersect(Ray& ray, Intersection& isec) {
    return scene_->intersect(ray, *this, isec);
}

inline bool Worker::intersect(Ray& ray, shape::Normals& normals) {
    return scene_->intersect(ray, *this, normals);
}

inline bool Worker::intersect_and_resolve_mask(Ray& ray, Intersection& isec, Filter filter) {
    if (!intersect(ray, isec)) {
        return false;
    }

    return resolve_mask(ray, isec, filter);
}

inline Result1 Worker::visibility(Ray const& ray, Filter filter) {
    return scene_->visibility(ray, filter, *this);
}

inline bool Worker::intersect_p(uint32_t prop, Ray const& ray) {
    return scene_->prop(prop)->intersect_p(prop, ray, *this);
}

inline Scene const& Worker::scene() const {
    return *scene_;
}

inline camera::Camera const& Worker::camera() const {
    return *camera_;
}

inline uint64_t Worker::absolute_time(uint32_t frame, float frame_delta) const {
    return camera_->absolute_time(frame, frame_delta);
}

inline shape::Node_stack& Worker::node_stack() {
    return node_stack_;
}

inline material::Sample_cache& Worker::sample_cache() {
    return sample_cache_;
}

inline image::texture::Texture const* Worker::texture(uint32_t id) const {
    return scene_->texture(id);
}

inline prop::Interface_stack& Worker::interface_stack() {
    return interface_stack_;
}

inline rnd::Generator& Worker::rng() {
    return rng_;
}

inline light::Tree::Lights& Worker::lights() {
    return lights_;
}

template <typename T>
T& Worker::sample() {
    return sample_cache_.get<T>();
}

}  // namespace scene

#endif
