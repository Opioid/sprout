#ifndef SU_CORE_SCENE_MATERIAL_VOLUMETRIC_HOMOGENEOUS_HPP
#define SU_CORE_SCENE_MATERIAL_VOLUMETRIC_HOMOGENEOUS_HPP

#include "volumetric_material.hpp"

namespace scene::material::volumetric {

class Homogeneous : public Material {
  public:
    Homogeneous(Sampler_settings const& sampler_settings) noexcept;

    float3 emission(math::Ray const& ray, Transformation const& transformation, float step_size,
                    rnd::Generator& rng, Filter filter, Worker const& worker) const
        noexcept override final;

    float3 absorption_coefficient(float2 uv, Filter filter, Worker const& worker) const
        noexcept override final;

    CC collision_coefficients() const noexcept override final;

    CC collision_coefficients(float2 uv, Filter filter, Worker const& worker) const
        noexcept override final;

    CC collision_coefficients(float3 const& p, Filter filter, Worker const& worker) const
        noexcept override final;

    CCE collision_coefficients_emission() const noexcept override final;

    size_t num_bytes() const noexcept override;
};

}  // namespace scene::material::volumetric

#endif
