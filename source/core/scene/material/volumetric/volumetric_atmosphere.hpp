#ifndef SU_CORE_SCENE_MATERIAL_VOLUMETRIC_ATMOSPHERE_HPP
#define SU_CORE_SCENE_MATERIAL_VOLUMETRIC_ATMOSPHERE_HPP

#include "volumetric_material.hpp"

namespace scene::material::volumetric {

class Atmosphere final : public Material {
  public:
    Atmosphere(Sampler_settings const& sampler_settings);

    virtual float3 emission(Transformation const& transformation, ray const& ray, float step_size,
                            rnd::Generator& rng, Filter filter, Worker const& worker) const final;

    virtual float3 optical_depth(Transformation const& transformation, AABB const& aabb,
                                 ray const& ray, float step_size, rnd::Generator& rng,
                                 Filter filter, Worker const& worker) const final;

    virtual float3 scattering(Transformation const& transformation, float3 const& p, Filter filter,
                              Worker const& worker) const final;

    virtual size_t num_bytes() const final;
};

}  // namespace scene::material::volumetric

#endif
