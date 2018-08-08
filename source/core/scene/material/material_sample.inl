#ifndef SU_SCENE_MATERIAL_SAMPLE_INL
#define SU_SCENE_MATERIAL_SAMPLE_INL

#include <cmath>
#include "base/math/vector3.inl"
#include "material_sample.hpp"
#include "material_sample_helper.hpp"
#include "scene/scene_worker.hpp"

namespace scene::material {

inline Sample::~Sample() {}

inline void Sample::Layer::set_tangent_frame(float3 const& t, float3 const& b,
                                             float3 const& n) noexcept {
    t_ = t;
    b_ = b;
    n_ = n;
}

inline void Sample::Layer::set_tangent_frame(float3 const& n) noexcept {
    math::orthonormal_basis(n, t_, b_);
    n_ = n;
}

inline float Sample::Layer::abs_n_dot(float3 const& v) const noexcept {
    return abs_dot(n_, v);
}

inline float Sample::Layer::clamp_n_dot(float3 const& v) const noexcept {
    // return std::max(math::dot(n, v), Dot_min);
    return clamp_dot(n_, v);
}

inline float Sample::Layer::clamp_abs_n_dot(float3 const& v) const noexcept {
    // return std::max(math::dot(n, v), Dot_min);
    return clamp_abs_dot(n_, v);
}

inline float Sample::Layer::clamp_reverse_n_dot(float3 const& v) const noexcept {
    // return std::max(-math::dot(n, v), Dot_min);
    return clamp_reverse_dot(n_, v);
}

inline float3 const& Sample::Layer::shading_normal() const noexcept {
    return n_;
}

inline float3 Sample::Layer::tangent_to_world(float3 const& v) const noexcept {
    return float3(v[0] * t_[0] + v[1] * b_[0] + v[2] * n_[0],
                  v[0] * t_[1] + v[1] * b_[1] + v[2] * n_[1],
                  v[0] * t_[2] + v[1] * b_[2] + v[2] * n_[2]);
}

inline float3 Sample::Layer::world_to_tangent(float3 const& v) const noexcept {
    return float3(v[0] * t_[0] + v[1] * t_[1] + v[2] * t_[2],
                  v[0] * b_[0] + v[1] * b_[1] + v[2] * b_[2],
                  v[0] * n_[0] + v[1] * n_[1] + v[2] * n_[2]);
}

inline float3 Sample::radiance() const noexcept {
    return float3::identity();
}

inline bool Sample::is_pure_emissive() const noexcept {
    return false;
}

inline bool Sample::is_translucent() const noexcept {
    return false;
}

inline bool Sample::ior_greater_one() const noexcept {
    return true;
}

inline float3 const& Sample::wo() const noexcept {
    return wo_;
}

inline float Sample::clamp_geo_n_dot(float3 const& v) const noexcept {
    return clamp_dot(geo_n_, v);
}

inline float Sample::clamp_reverse_geo_n_dot(float3 const& v) const noexcept {
    return clamp_reverse_dot(geo_n_, v);
}

inline float3 const& Sample::geometric_normal() const noexcept {
    return geo_n_;
}

inline bool Sample::same_hemisphere(float3 const& v) const noexcept {
    return math::dot(geo_n_, v) > 0.f;
}

inline void Sample::set_basis(float3 const& geo_n, float3 const& wo) noexcept {
    geo_n_ = geo_n;
    wo_    = wo;
}

}  // namespace scene::material

#endif
