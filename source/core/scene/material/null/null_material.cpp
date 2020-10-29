#include "null_material.hpp"
#include "base/math/vector3.inl"
#include "null_sample.hpp"
#include "scene/material/material_sample.inl"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.inl"

namespace scene::material::null {

inline float3 constexpr Color_front(0.4f, 0.9f, 0.1f);
inline float3 constexpr Color_back(0.9f, 0.1f, 0.4f);

Material::Material(Sampler_settings const& sampler_settings)
    : material::Material(sampler_settings, true) {}

material::Sample const& Material::sample(float3 const&      wo, Ray const& /*ray*/,
                                         Renderstate const& rs, Filter /*filter*/,
                                         Sampler& /*sampler*/, Worker& worker) const {
    auto& sample = worker.sample<Sample>();

    sample.set_common(rs.geo_n, rs.n, wo, float3(0.f), float3(0.f), 1.f);

    sample.layer_.set_tangent_frame(rs.t, rs.b, rs.n);

    return sample;
}

}  // namespace scene::material::null
