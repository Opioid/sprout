#ifndef SU_CORE_SCENE_MATERIAL_VOLUMETRIC_DENSITY_HPP
#define SU_CORE_SCENE_MATERIAL_VOLUMETRIC_DENSITY_HPP

#include "volumetric_material.hpp"

namespace scene::material::volumetric {

class Density : public Material {
  public:
    Density(Sampler_settings const& sampler_settings) noexcept;

    CC collision_coefficients() const noexcept override final;

    CC collision_coefficients(float2 uv, Filter filter, Worker const& worker) const
        noexcept override final;

    CC collision_coefficients(float3 const& uvw, Filter filter, Worker const& worker) const
        noexcept override final;

    CCE collision_coefficients_emission(float3 const& uvw, Filter filter,
                                        Worker const& worker) const noexcept override final;

  private:
    // expects p in object space!
    virtual float density(float3 const& p, Filter filter, Worker const& worker) const noexcept = 0;
};

}  // namespace scene::material::volumetric

#endif
