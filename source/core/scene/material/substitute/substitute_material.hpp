#ifndef SU_CORE_SCENE_MATERIAL_SUBSTITUTE_MATERIAL_HPP
#define SU_CORE_SCENE_MATERIAL_SUBSTITUTE_MATERIAL_HPP

#include "substitute_base_material.hpp"

namespace scene::material::substitute {

class alignas(32) Material : public Material_base {
  public:
    Material(Sampler_settings const& sampler_settings, bool two_sided) noexcept;

    material::Sample const& sample(float3 const& wo, Ray const& ray, Renderstate const& rs,
                                   Filter filter, Sampler& sampler, Worker const& worker) const
        noexcept final;

    size_t num_bytes() const noexcept final;

    static size_t sample_size() noexcept;
};

class alignas(32) Frozen : public Material_base {
  public:
    Frozen(Sampler_settings const& sampler_settings, bool two_sided) noexcept;

    material::Sample const& sample(float3 const& wo, Ray const& ray, Renderstate const& rs,
                                   Filter filter, Sampler& sampler, Worker const& worker) const
        noexcept final;

    void set_snow_normal_map(Texture_adapter const& normal_map) noexcept;

    void set_snow_mask(Texture_adapter const& mask) noexcept;

    size_t num_bytes() const noexcept final;

    static size_t sample_size() noexcept;

  private:
    Texture_adapter snow_normal_map_;

    Texture_adapter snow_mask_;
};

}  // namespace scene::material::substitute

#endif
