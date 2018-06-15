#include "debug_material.hpp"
#include "base/math/vector3.inl"
#include "debug_sample.hpp"
#include "image/texture/sampler/sampler_2d.hpp"
#include "scene/material/material_sample.inl"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.inl"

namespace scene::material::debug {

Material::Material(Sampler_settings const& sampler_settings)
    : material::Material(sampler_settings, true) {}

const material::Sample& Material::sample(f_float3 wo, Renderstate const& rs,
                                         Sampler_filter /*filter*/, sampler::Sampler& /*sampler*/,
                                         Worker const& worker, uint32_t depth) const {
    auto& sample = worker.sample<Sample>(depth);

    sample.set_basis(rs.geo_n, wo);

    sample.layer_.set_tangent_frame(rs.t, rs.b, rs.n);

    return sample;
}

float Material::ior() const {
    return 1.47f;
}

size_t Material::num_bytes() const {
    return sizeof(*this);
}

}  // namespace scene::material::debug
