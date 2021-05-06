#include "debug_material.hpp"
#include "base/math/vector3.inl"
#include "debug_sample.hpp"
#include "scene/material/material_sample.inl"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.inl"

namespace scene::material::debug {

inline float3 constexpr Color_front(0.4f, 0.9f, 0.1f);
inline float3 constexpr Color_back(0.9f, 0.1f, 0.4f);

Material::Material(Sampler_settings sampler_settings)
    : material::Material(sampler_settings, true) {}

material::Sample const& Material::sample(float3_p wo, Renderstate const& rs, Sampler& /*sampler*/,
                                         Worker& worker) const {
    auto& sample = worker.sample<Sample>();

    float3 const n = cross(rs.t, rs.b);

    bool const same_side = dot(n, rs.n) > 0.f;

    float3 const color = same_side ? Color_front : Color_back;

    sample.set_common(rs, wo, color, float3(0.f), 1.f);

    sample.layer_.set_tangent_frame(rs.t, rs.b, rs.n);

    return sample;
}

}  // namespace scene::material::debug
