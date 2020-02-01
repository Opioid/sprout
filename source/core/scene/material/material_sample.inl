#ifndef SU_SCENE_MATERIAL_SAMPLE_INL
#define SU_SCENE_MATERIAL_SAMPLE_INL

#include "base/math/vector3.inl"
#include "material_sample.hpp"
#include "material_sample_helper.hpp"
#include "scene/scene_constants.hpp"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.hpp"

#include <cmath>

namespace scene::material {

inline Layer Layer::swapped() const noexcept {
    return {t_, b_, -n_};
}

inline Layer Layer::swapped(bool same_side) const noexcept {
    if (same_side) {
        return *this;
    }

    return {t_, b_, -n_};
}

inline void Layer::set_tangent_frame(float3 const& t, float3 const& b, float3 const& n) noexcept {
    t_ = t;
    b_ = b;
    n_ = n;
}

inline void Layer::set_tangent_frame(float3 const& n) noexcept {
    auto const [t, b] = orthonormal_basis(n);

    t_ = t;
    b_ = b;
    n_ = n;
}

inline float Layer::n_dot(float3 const& v) const noexcept {
    return dot(n_, v);
}

inline float Layer::abs_n_dot(float3 const& v) const noexcept {
    return abs_dot(n_, v);
}

inline float Layer::clamp_n_dot(float3 const& v) const noexcept {
    // return std::max(dot(n, v), Dot_min);
    return clamp_dot(n_, v);
}

inline float Layer::clamp_abs_n_dot(float3 const& v) const noexcept {
    // return std::max(dot(n, v), Dot_min);
    return clamp_abs_dot(n_, v);
}

inline float3 const& Layer::shading_normal() const noexcept {
    return n_;
}

inline float3 Layer::tangent_to_world(float3 const& v) const noexcept {
    return float3(v[0] * t_[0] + v[1] * b_[0] + v[2] * n_[0],
                  v[0] * t_[1] + v[1] * b_[1] + v[2] * n_[1],
                  v[0] * t_[2] + v[1] * b_[2] + v[2] * n_[2]);
}

inline float3 Layer::world_to_tangent(float3 const& v) const noexcept {
    return float3(v[0] * t_[0] + v[1] * t_[1] + v[2] * t_[2],
                  v[0] * b_[0] + v[1] * b_[1] + v[2] * b_[2],
                  v[0] * n_[0] + v[1] * n_[1] + v[2] * n_[2]);
}

inline Sample::~Sample() {}

inline float3 Sample::radiance() const noexcept {
    return float3(0.f);
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

inline bool Sample::evaluates_back(bool /*previously*/, bool /*same_side*/) const noexcept {
    return true;
}

inline float3 Sample::offset_p(float3 const& p) const noexcept {
    return offset_ray(p, geo_n_);
}

inline float3 Sample::offset_p(float3 const& p, float3 const& wi) const noexcept {
    return offset_ray(p, same_hemisphere(wi) ? geo_n_ : -geo_n_);
}

inline float3 const& Sample::wo() const noexcept {
    return wo_;
}

inline float Sample::clamp_geo_n_dot(float3 const& v) const noexcept {
    return clamp_dot(geo_n_, v);
}

inline float3 const& Sample::geometric_normal() const noexcept {
    return geo_n_;
}

inline bool Sample::same_hemisphere(float3 const& v) const noexcept {
    return dot(geo_n_, v) > 0.f;
}

inline void Sample::set_basis(float3 const& geo_n, float3 const& wo) noexcept {
    geo_n_ = geo_n;
    wo_    = wo;
}

inline IoR IoR::swapped() const noexcept {
    return IoR{eta_i, eta_t};
}

inline IoR IoR::swapped(bool same_side) const noexcept {
    if (same_side) {
        return *this;
    }

    return IoR{eta_i, eta_t};
}

}  // namespace scene::material

#endif
