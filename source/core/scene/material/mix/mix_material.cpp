#include "mix_material.hpp"
#include "image/texture/texture_adapter.inl"
#include "sampler/sampler.hpp"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.inl"

namespace scene::material::mix {

Material::Material(Sampler_settings const& sampler_settings, bool two_sided)
    : material::Material(sampler_settings, two_sided) {}

const material::Sample& Material::sample(f_float3 wo, Renderstate const& rs, Sampler_filter filter,
                                         sampler::Sampler& sampler, Worker const& worker,
                                         uint32_t depth) const {
    auto&       texture_sampler = worker.sampler_2D(sampler_key(), filter);
    float const mask            = mask_.sample_1(texture_sampler, rs.uv);

    if (mask > sampler.generate_sample_1D(1)) {
        return material_a_->sample(wo, rs, filter, sampler, worker, depth);
    } else {
        return material_b_->sample(wo, rs, filter, sampler, worker, depth);
    }
}

float Material::opacity(float2 /*uv*/, float /*time*/, Sampler_filter /*filter*/,
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

void Material::set_materials(Material_ptr const& a, Material_ptr const& b) {
    material_a_ = a;
    material_b_ = b;
}

}  // namespace scene::material::mix
