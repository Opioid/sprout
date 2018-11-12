#ifndef SU_CORE_SCENE_MATERIAL_SUBSTITUTE_MATERIAL_HPP
#define SU_CORE_SCENE_MATERIAL_SUBSTITUTE_MATERIAL_HPP

#include "substitute_base_material.hpp"

namespace scene::material::substitute {

class Material : public Material_base {
  public:
    Material(Sampler_settings const& sampler_settings, bool two_sided) noexcept;

    material::Sample const& sample(float3 const& wo, Ray const& ray, Renderstate const& rs,
                                   Filter filter, sampler::Sampler& sampler, Worker const& worker,
                                   uint32_t sample_level) const noexcept override final;

    size_t num_bytes() const noexcept override final;

    static size_t sample_size() noexcept;
};

}  // namespace scene::material::substitute

#endif
