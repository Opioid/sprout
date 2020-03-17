#include "debug_material.hpp"
#include "base/math/vector3.inl"
#include "debug_sample.hpp"
#include "scene/material/material_sample.inl"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.inl"

namespace scene::material::debug {

Material::Material(Sampler_settings const& sampler_settings)
    : material::Material(sampler_settings, true) {}

material::Sample const& Material::sample(float3 const&      wo, Ray const& /*ray*/,
                                         Renderstate const& rs, Filter /*filter*/,
                                         Sampler& /*sampler*/, Worker& worker) const {
    auto& sample = worker.sample<Sample>();

    sample.set_basis(rs.geo_n, wo);

    sample.layer_.set_tangent_frame(rs.t, rs.b, rs.n);

    return sample;
}

size_t Material::num_bytes() const {
    return sizeof(*this);
}

}  // namespace scene::material::debug
