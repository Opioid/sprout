#include "mix_material.hpp"
#include "image/texture/texture_adapter.inl"
#include "sampler/sampler.hpp"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.inl"

namespace scene::material::mix {

Material::Material(Sampler_settings const& sampler_settings, bool two_sided)
    : material::Material(sampler_settings, two_sided) {}

material::Sample const& Material::sample(float3 const& wo, Ray const& ray, Renderstate const& rs,
                                         Filter filter, Sampler& sampler, Worker& worker) const {
    auto& texture_sampler = worker.sampler_2D(sampler_key(), filter);

    float const mask = mask_.sample_1(worker, texture_sampler, rs.uv);

    if (mask > sampler.generate_sample_1D(worker.rng(), 1)) {
        return material_a_->sample(wo, ray, rs, filter, sampler, worker);
    }

    return material_b_->sample(wo, ray, rs, filter, sampler, worker);
}

void Material::set_materials(material::Material const* a, material::Material const* b) {
    material_a_ = a;
    material_b_ = b;

    ior_ = a->ior();
}

}  // namespace scene::material::mix
