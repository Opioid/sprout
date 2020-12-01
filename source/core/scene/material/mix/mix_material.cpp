#include "mix_material.hpp"
#include "image/texture/texture_adapter.inl"
#include "sampler/sampler.hpp"
#include "scene/material/material.inl"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.inl"

namespace scene::material::mix {

Material::Material(Sampler_settings sampler_settings, bool two_sided)
    : material::Material(sampler_settings, two_sided) {}

material::Sample const& Material::sample(float3_p wo, Ray const& ray, Renderstate const& rs,
                                         Filter filter, Sampler& sampler, Worker& worker) const {
    auto& texture_sampler = worker.sampler_2D(sampler_key(), filter);

    float const mask = mask_.sample_1(worker, texture_sampler, rs.uv);

    if (mask > sampler.sample_1D(worker.rng(), 1)) {
        return worker.scene().material(material_a_)->sample(wo, ray, rs, filter, sampler, worker);
    }

    return worker.scene().material(material_a_)->sample(wo, ray, rs, filter, sampler, worker);
}

void Material::set_materials(uint32_t a, uint32_t b) {
    material_a_ = a;
    material_b_ = b;

    ior_ = 1.5f;
}

}  // namespace scene::material::mix
