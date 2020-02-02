#include "mix_material.hpp"
#include "image/texture/texture_adapter.inl"
#include "sampler/sampler.hpp"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.inl"

namespace scene::material::mix {

Material::Material(Sampler_settings const& sampler_settings, bool two_sided)
    : material::Material(sampler_settings, two_sided) {}

material::Sample const& Material::sample(float3 const& wo, Ray const& ray, Renderstate const& rs,
                                         Filter filter, Sampler& sampler,
                                         Worker const& worker) const {
    auto& texture_sampler = worker.sampler_2D(sampler_key(), filter);

    float const mask = mask_.sample_1(worker, texture_sampler, rs.uv);

    if (mask > sampler.generate_sample_1D(1)) {
        return material_a_->sample(wo, ray, rs, filter, sampler, worker);
    }

    return material_b_->sample(wo, ray, rs, filter, sampler, worker);
}

float Material::opacity(float2 /*uv*/, uint64_t /*time*/, Filter /*filter*/,
                        Worker const& /*worker*/) const {
    return 1.f;
}

bool Material::is_masked() const {
    return material_a_->is_masked() || material_b_->is_masked();
}

float Material::ior() const {
    return material_a_->ior();
}

size_t Material::num_bytes() const {
    return sizeof(*this);
}

void Material::set_materials(material::Material const* a, material::Material const* b) {
    material_a_ = a;
    material_b_ = b;
}

}  // namespace scene::material::mix
