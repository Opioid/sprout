#include "matte_material.hpp"
#include "base/math/vector3.inl"
#include "image/texture/sampler/sampler_2d.hpp"
#include "matte_sample.hpp"
#include "scene/material/material_sample.inl"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.inl"

namespace scene::material::matte {

Material::Material(Sampler_settings const& sampler_settings, bool two_sided) noexcept
    : material::Material(sampler_settings, two_sided) {}

material::Sample const& Material::sample(float3 const& wo, Renderstate const& rs, Filter /*filter*/,
                                         sampler::Sampler& /*sampler*/, Worker const& worker,
                                         uint32_t depth) const noexcept {
    auto& sample = worker.sample<Sample>(depth);

    //	auto& sampler = worker.sampler_2D(sampler_key_, filter);

    sample.set_basis(rs.geo_n, wo);

    sample.layer_.set_tangent_frame(rs.t, rs.b, rs.n);

    sample.set(color_);

    return sample;
}

float Material::ior() const noexcept {
    return 1.47f;
}

size_t Material::num_bytes() const noexcept {
    return sizeof(*this);
}

void Material::set_color(float3 const& color) noexcept {
    color_ = color;
}

size_t Material::sample_size() noexcept {
    return sizeof(Sample);
}

}  // namespace scene::material::matte
