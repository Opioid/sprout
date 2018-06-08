#ifndef SU_CORE_SCENE_MATERIAL_SUBSTITUTE_MATERIAL_HPP
#define SU_CORE_SCENE_MATERIAL_SUBSTITUTE_MATERIAL_HPP

#include "substitute_base_material.hpp"

namespace scene::material::substitute {

class Material : public Material_base {
  public:
    Material(Sampler_settings const& sampler_settings, bool two_sided);

    virtual const material::Sample& sample(f_float3 wo, Renderstate const& rs,
                                           Sampler_filter filter, sampler::Sampler& sampler,
                                           Worker const& worker) const override final;

    virtual size_t num_bytes() const override final;

    static size_t sample_size();
};

}  // namespace scene::material::substitute

#endif
