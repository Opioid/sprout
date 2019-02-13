#ifndef SU_SCENE_MATERIAL_SAMPLE_INL
#define SU_SCENE_MATERIAL_SAMPLE_INL

#include <cmath>
#include "base/math/vector3.inl"
#include "material_sample.hpp"
#include "material_sample_helper.hpp"
#include "scene/scene_worker.hpp"
#include "scene/scene_renderstate.hpp"
#include <cstring>

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

inline float Layer::clamp_reverse_n_dot(float3 const& v) const noexcept {
    // return std::max(-dot(n, v), Dot_min);
    return clamp_reverse_dot(n_, v);
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

inline bool Sample::do_evaluate_back(bool /*previously*/, bool /*same_side*/) const noexcept {
    return true;
}

static inline float int_as_float(int32_t x) noexcept {
    float f;
    std::memcpy(&f, &x, sizeof(float));
    return f;
}

static inline int32_t float_as_int(float x) noexcept {
    int32_t i;
    std::memcpy(&i, &x, sizeof(int32_t));
    return i;
}


static float constexpr origin = 1.f / 32.f;
static float constexpr float_scale = 1.f / 65536.f;
static float constexpr int_scale = 256.f;

static inline float3 offset_ray(float3 const& p, float3 const& n) noexcept {
    int3 const of_i(int_scale * n);

    float3 const p_i(int_as_float(float_as_int(p[0]) + ((p[0] < 0.f) ? -of_i[0] : of_i[0] )),
                     int_as_float(float_as_int(p[1]) + ((p[1] < 0.f) ? -of_i[1] : of_i[1] )),
                     int_as_float(float_as_int(p[2]) + ((p[2] < 0.f) ? -of_i[2] : of_i[2] )));

    return float3(std::abs(p[0]) < origin ? p[0] + float_scale * n[0] : p_i[0],
                  std::abs(p[1]) < origin ? p[1] + float_scale * n[1] : p_i[1],
                  std::abs(p[2]) < origin ? p[2] + float_scale * n[2] : p_i[2]);
}


inline float3 Sample::p() const noexcept {


    return offset_ray(p_, geo_n_);

}

inline float3 Sample::p(float3 const& wi) const noexcept {


    return offset_ray(p_, same_hemisphere(wi) ? geo_n_ : -geo_n_);

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
    return dot(geo_n_, v) > 0.f;
}

inline void Sample::set_basis(Renderstate const& rs, float3 const& wo) noexcept {
    p_ = rs.p;
    geo_n_ = rs.geo_n;
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
