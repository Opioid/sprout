#include "mix_material.hpp"
#include "image/texture/texture_adapter.inl"
#include "sampler/sampler.hpp"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.inl"

namespace scene::material::mix {

Material::Material(Sampler_settings const& sampler_settings, bool two_sided) noexcept
    : material::Material(sampler_settings, two_sided) {}

material::Sample const& Material::sample(float3 const& wo, Ray const& ray, Renderstate const& rs,
                                         Filter filter, sampler::Sampler& sampler,
                                         Worker const& worker) const noexcept {
    auto& texture_sampler = worker.sampler_2D(sampler_key(), filter);

    float const mask = mask_.sample_1(texture_sampler, rs.uv);

    if (mask > sampler.generate_sample_1D(1)) {
        return material_a_->sample(wo, ray, rs, filter, sampler, worker);
    } else {
        return material_b_->sample(wo, ray, rs, filter, sampler, worker);
    }
}

float Material::opacity(float2 /*uv*/, uint64_t /*time*/, Filter /*filter*/,
                        Worker const& /*worker*/) const noexcept {
    return 1.f;
}

bool Material::is_masked() const noexcept {
    return material_a_->is_masked() || material_b_->is_masked();
}

float Material::ior() const noexcept {
    return material_a_->ior();
}

size_t Material::num_bytes() const noexcept {
    return sizeof(*this);
}

void Material::set_materials(Material_ptr const& a, Material_ptr const& b) noexcept {
    material_a_ = a;
    material_b_ = b;
}

}  // namespace scene::material::mix
