#ifndef SU_SCENE_MATERIAL_SAMPLE_INL
#define SU_SCENE_MATERIAL_SAMPLE_INL

#include <cmath>
#include "base/math/vector3.inl"
#include "material_sample.hpp"
#include "material_sample_helper.hpp"
#include "scene/scene_worker.hpp"

namespace scene::material {

inline Sample::~Sample() {}

inline void Sample::Layer::set_tangent_frame(f_float3 t, f_float3 b, f_float3 n) {
    t_ = t;
    b_ = b;
    n_ = n;
}

inline void Sample::Layer::set_tangent_frame(f_float3 n) {
    math::orthonormal_basis(n, t_, b_);
    n_ = n;
}

inline float Sample::Layer::abs_n_dot(f_float3 v) const {
    return abs_dot(n_, v);
}

inline float Sample::Layer::clamp_n_dot(f_float3 v) const {
    // return std::max(math::dot(n, v), Dot_min);
    return clamp_dot(n_, v);
}

inline float Sample::Layer::clamp_abs_n_dot(f_float3 v) const {
    // return std::max(math::dot(n, v), Dot_min);
    return clamp_abs_dot(n_, v);
}

inline float Sample::Layer::clamp_reverse_n_dot(f_float3 v) const {
    // return std::max(-math::dot(n, v), Dot_min);
    return clamp_reverse_dot(n_, v);
}

inline f_float3 Sample::Layer::shading_normal() const {
    return n_;
}

inline float3 Sample::Layer::tangent_to_world(f_float3 v) const {
    return float3(v[0] * t_[0] + v[1] * b_[0] + v[2] * n_[0],
                  v[0] * t_[1] + v[1] * b_[1] + v[2] * n_[1],
                  v[0] * t_[2] + v[1] * b_[2] + v[2] * n_[2]);
}

inline float3 Sample::Layer::world_to_tangent(f_float3 v) const {
    return float3(v[0] * t_[0] + v[1] * t_[1] + v[2] * t_[2],
                  v[0] * b_[0] + v[1] * b_[1] + v[2] * b_[2],
                  v[0] * n_[0] + v[1] * n_[1] + v[2] * n_[2]);
}

inline float3 Sample::radiance() const {
    return float3::identity();
}

inline bool Sample::is_pure_emissive() const {
    return false;
}

inline bool Sample::is_transmissive() const {
    return false;
}

inline bool Sample::is_translucent() const {
    return false;
}

inline bool Sample::ior_greater_one() const {
    return true;
}

inline float Sample::photon_n_dot(f_float3 wi) const {
    return base_layer().abs_n_dot(wi);
}

inline f_float3 Sample::wo() const {
    return wo_;
}

inline float Sample::clamp_geo_n_dot(f_float3 v) const {
    return clamp_dot(geo_n_, v);
}

inline float Sample::clamp_reverse_geo_n_dot(f_float3 v) const {
    return clamp_reverse_dot(geo_n_, v);
}

inline f_float3 Sample::geometric_normal() const {
    return geo_n_;
}

inline bool Sample::same_hemisphere(f_float3 v) const {
    return math::dot(geo_n_, v) > 0.f;
}

inline void Sample::set_basis(f_float3 geo_n, f_float3 wo) {
    geo_n_ = geo_n;
    wo_    = wo;
}

}  // namespace scene::material

#endif
