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

inline Layer Layer::swapped() const {
    return {t_, b_, -n_};
}

inline Layer Layer::swapped(bool same_side) const {
    if (same_side) {
        return *this;
    }

    return {t_, b_, -n_};
}

inline void Layer::set_tangent_frame(float3_p t, float3_p b, float3_p n) {
    t_ = t;
    b_ = b;
    n_ = n;
}

inline void Layer::set_tangent_frame(float3_p n) {
    auto const [t, b] = orthonormal_basis(n);

    t_ = t;
    b_ = b;
    n_ = n;
}

inline float Layer::n_dot(float3_p v) const {
    return dot(n_, v);
}

inline float Layer::abs_n_dot(float3_p v) const {
    return abs_dot(n_, v);
}

inline float Layer::clamp_n_dot(float3_p v) const {
    // return std::max(dot(n, v), Dot_min);
    return clamp_dot(n_, v);
}

inline float Layer::clamp_abs_n_dot(float3_p v) const {
    // return std::max(dot(n, v), Dot_min);
    return clamp_abs_dot(n_, v);
}

inline float3 Layer::shading_normal() const {
    return n_;
}

inline float3 Layer::tangent_to_world(float3_p v) const {
    return float3(v[0] * t_[0] + v[1] * b_[0] + v[2] * n_[0],
                  v[0] * t_[1] + v[1] * b_[1] + v[2] * n_[1],
                  v[0] * t_[2] + v[1] * b_[2] + v[2] * n_[2]);
}

inline float3 Layer::world_to_tangent(float3_p v) const {
    return float3(v[0] * t_[0] + v[1] * t_[1] + v[2] * t_[2],
                  v[0] * b_[0] + v[1] * b_[1] + v[2] * b_[2],
                  v[0] * n_[0] + v[1] * n_[1] + v[2] * n_[2]);
}

inline Sample::Sample() : properties_(Property::Can_evaluate) {}

inline Sample::~Sample() = default;

inline bool Sample::is_pure_emissive() const {
    return properties_.is(Property::Pure_emissive);
}

inline bool Sample::is_translucent() const {
    return properties_.is(Property::Translucent);
}

inline bool Sample::can_evaluate() const {
    return properties_.is(Property::Can_evaluate);
}

inline bool Sample::avoid_caustics() const {
    return properties_.is(Property::Avoid_caustics);
}

inline float3 Sample::offset_p(float3_p p, bool subsurface, bool translucent) const {
    if (subsurface) {
        return float3(p[0], p[1], p[2], 0.f);
    }

    if (translucent) {
        return float3(p[0], p[1], p[2], Float_scale);
    }

    return offset_ray(p, geo_n_);
}

inline float3 Sample::offset_p(float3_p p, float3_p wi, bool subsurface) const {
    if (subsurface) {
        return float3(p[0], p[1], p[2], 0.f);
    }

    return offset_ray(p, same_hemisphere(wi) ? geo_n_ : -geo_n_);
}

inline float3 Sample::geometric_normal() const {
    return geo_n_;
}

inline float3 Sample::interpolated_normal() const {
    return n_;
}

inline float3 Sample::shading_normal() const {
    return layer_.n_;
}

inline float3 Sample::shading_tangent() const {
    return layer_.t_;
}

inline float3 Sample::shading_bitangent() const {
    return layer_.b_;
}

inline float3 Sample::wo() const {
    return wo_;
}

inline float3 Sample::albedo() const {
    return albedo_;
}

inline float3 Sample::radiance() const {
    return radiance_;
}

inline float Sample::alpha() const {
    return alpha_;
}

inline float Sample::clamp_geo_n_dot(float3_p v) const {
    return clamp_dot(geo_n_, v);
}

inline bool Sample::same_hemisphere(float3_p v) const {
    return dot(geo_n_, v) >= 0.f;
}

inline void Sample::set_common(Renderstate const& rs, float3_p wo, float3_p albedo,
                               float3_p radiance, float alpha) {
    geo_n_    = rs.geo_n;
    n_        = rs.n;
    wo_       = wo;
    albedo_   = albedo;
    radiance_ = radiance;
    alpha_    = alpha;

    properties_.set(Property::Avoid_caustics, rs.avoid_caustics);
}

inline void Sample::set_common(float3_p geo_n, float3_p n, float3_p wo, float alpha) {
    geo_n_    = geo_n;
    n_        = n;
    wo_       = wo;
    albedo_   = float3(0.f);
    radiance_ = float3(0.f);
    alpha_    = alpha;
}

inline void Sample::set_radiance(float3_p radiance) {
    radiance_ = radiance;
}

inline IoR IoR::swapped() const {
    return IoR{eta_i, eta_t};
}

inline IoR IoR::swapped(bool same_side) const {
    if (same_side) {
        return *this;
    }

    return IoR{eta_i, eta_t};
}

}  // namespace scene::material

#endif
