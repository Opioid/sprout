#ifndef SU_EXTENSION_PROCEDURAL_SKY_SUN_MATERIAL_HPP
#define SU_EXTENSION_PROCEDURAL_SKY_SUN_MATERIAL_HPP

#include "base/math/interpolated_function_1d.hpp"
#include "sky_material_base.hpp"

namespace procedural::sky {

class Sun_material : public Material {
  public:
    Sun_material(Sky* sky);

    scene::material::Sample const& sample(float3_p wo, scene::Renderstate const& rs,
                                          Sampler& sampler, scene::Worker& worker) const final;

    float3 evaluate_radiance(float3_p wi, float3_p n, float3_p uvw, float extent, Filter filter,
                             scene::Worker const& worker) const final;

    float3 prepare_sampling(Shape const& shape, uint32_t part, Transformation const& trafo,
                            float area, scene::Scene const& scene, Threads& threads) final;
};

class Sun_baked_material : public Material {
  public:
    Sun_baked_material(Sky* sky);

    scene::material::Sample const& sample(float3_p wo, scene::Renderstate const& rs,
                                          Sampler& sampler, scene::Worker& worker) const final;

    float3 evaluate_radiance(float3_p wi, float3_p n, float3_p uvw, float extent, Filter filter,
                             scene::Worker const& worker) const final;

    float3 prepare_sampling(Shape const& shape, uint32_t part, Transformation const& trafo,
                            float area, scene::Scene const& scene, Threads& threads) final;

  private:
    math::Interpolated_function_1D<float3> sun_emission_;
};

}  // namespace procedural::sky

#endif
