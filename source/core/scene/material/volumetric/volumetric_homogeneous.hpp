#ifndef SU_CORE_SCENE_MATERIAL_VOLUMETRIC_HOMOGENEOUS_HPP
#define SU_CORE_SCENE_MATERIAL_VOLUMETRIC_HOMOGENEOUS_HPP

#include "volumetric_material.hpp"

namespace scene::material::volumetric {

class Homogeneous : public Material {
  public:
    Homogeneous(Sampler_settings const& sampler_settings);

    void compile(thread::Pool& threads, Scene const& scene) final;

    float3 evaluate_radiance(float3 const& wi, float3 const& uvw, float volume, Filter filter,
                             Worker const& worker) const final;

    float3 absorption_coefficient(float2 uv, Filter filter, Worker const& worker) const final;

    CC collision_coefficients() const final;

    CC collision_coefficients(float2 uv, Filter filter, Worker const& worker) const final;

    CC collision_coefficients(float3 const& uvw, Filter filter, Worker const& worker) const final;

    CCE collision_coefficients_emission() const final;

    CCE collision_coefficients_emission(float3 const& uvw, Filter filter,
                                        Worker const& worker) const final;

    size_t num_bytes() const override;
};

}  // namespace scene::material::volumetric

#endif
