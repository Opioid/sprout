#include "debug_material.hpp"
#include "base/math/vector3.inl"
#include "debug_sample.hpp"
#include "image/texture/sampler/sampler_2d.hpp"
#include "scene/material/material_sample.inl"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.inl"

namespace scene::material::debug {

Material::Material(Sampler_settings const& sampler_settings) noexcept
    : material::Material(sampler_settings, true) {}

material::Sample const& Material::sample(float3 const&      wo, Ray const& /*ray*/,
                                         Renderstate const& rs, Filter /*filter*/,
                                         sampler::Sampler& /*sampler*/, Worker const& worker) const
    noexcept {
    auto& sample = worker.sample<Sample>(rs.sample_level);

    sample.set_basis(rs, wo);

    sample.layer_.set_tangent_frame(rs.t, rs.b, rs.n);

    return sample;
}

float Material::ior() const noexcept {
    return 1.47f;
}

size_t Material::num_bytes() const noexcept {
    return sizeof(*this);
}

}  // namespace scene::material::debug
