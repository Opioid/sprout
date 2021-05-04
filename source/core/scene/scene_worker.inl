#ifndef SU_CORE_SCENE_WORKER_INL
#define SU_CORE_SCENE_WORKER_INL

#include "camera/camera.hpp"
#include "material/material_sample_cache.inl"
#include "scene/scene.inl"
#include "scene_worker.hpp"

#include <bit>

namespace scene {

inline bool Worker::intersect(Ray& ray, Intersection& isec) {
    return scene_->intersect(ray, *this, isec);
}

inline bool Worker::intersect(Ray& ray, shape::Normals& normals) {
    return scene_->intersect(ray, *this, normals);
}

inline bool Worker::intersect(uint32_t prop, Ray& ray, shape::Normals& normals) {
    return scene_->prop(prop)->intersect(prop, ray, *this, normals);
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

inline prop::Interface_stack& Worker::interface_stack() {
    return interface_stack_;
}

inline rnd::Generator& Worker::rng() {
    return rng_;
}

inline float3 Worker::random_float3_10bit() {
    uint32_t const r = rng_.random_uint();

    uint32_t bits0 = r >> 8;
    uint32_t bits1 = r << 2;
    uint32_t bits2 = r << 12;

    bits0 &= 0x007FE000u;
    bits1 &= 0x007FE000u;
    bits2 &= 0x007FE000u;

    bits0 |= 0x3F800000u;
    bits1 |= 0x3F800000u;
    bits2 |= 0x3F800000u;

    return float3(std::bit_cast<float>(bits0) * 1.0004885196685791015625f - 1.f,
                  std::bit_cast<float>(bits1) * 1.0004885196685791015625f - 1.f,
                  std::bit_cast<float>(bits2) * 1.0004885196685791015625f - 1.f);
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
