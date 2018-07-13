#ifndef SU_CORE_SCENE_MATERIAL_VOLUMETRIC_HOMOGENEOUS_HPP
#define SU_CORE_SCENE_MATERIAL_VOLUMETRIC_HOMOGENEOUS_HPP

#include "volumetric_material.hpp"

namespace scene::material::volumetric {

class Homogeneous final : public Material {
  public:
    Homogeneous(Sampler_settings const& sampler_settings);

    virtual float3 emission(math::Ray const& ray, Transformation const& transformation,
                            float step_size, rnd::Generator& rng, Sampler_filter filter,
                            Worker const& worker) const override final;

    virtual float3 absorption_coefficient(float2 uv, Sampler_filter filter,
                                          Worker const& worker) const override final;

    virtual CC collision_coefficients() const override final;

    virtual CC collision_coefficients(float2 uv, Sampler_filter filter,
                                      Worker const& worker) const override final;

    virtual CC collision_coefficients(f_float3 p, Sampler_filter filter,
                                      Worker const& worker) const override final;

    virtual float majorant_mu_t() const override final;

    virtual size_t num_bytes() const override final;
};

}  // namespace scene::material::volumetric

#endif
