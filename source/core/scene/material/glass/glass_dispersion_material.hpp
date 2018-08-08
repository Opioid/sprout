#ifndef SU_CORE_SCENE_MATERIAL_GLASS_DISPERSION_MATERIAL_HPP
#define SU_CORE_SCENE_MATERIAL_GLASS_DISPERSION_MATERIAL_HPP

#include "glass_material.hpp"

namespace scene::material::glass {

class Glass_dispersion final : public Glass {
  public:
    Glass_dispersion(Sampler_settings const& sampler_settings) noexcept;

    material::Sample const& sample(float3 const& wo, Renderstate const& rs, Sampler_filter filter,
                                   sampler::Sampler& sampler, Worker const& worker,
                                   uint32_t depth) const noexcept override final;

    size_t num_bytes() const noexcept override final;

    void set_abbe(float abbe) noexcept;

    static size_t sample_size() noexcept;

  private:
    float abbe_;
};

}  // namespace scene::material::glass

#endif
