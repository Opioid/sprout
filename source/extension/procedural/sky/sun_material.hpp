#ifndef SU_EXTENSION_PROCEDURAL_SKY_SUN_MATERIAL_HPP
#define SU_EXTENSION_PROCEDURAL_SKY_SUN_MATERIAL_HPP

#include "base/math/interpolated_function_1d.hpp"
#include "sky_material_base.hpp"

namespace procedural::sky {

class Sun_material : public Material {
  public:
    Sun_material(Sky* sky);

    scene::material::Sample const& sample(float3_p wo, scene::Ray const& ray,
                                          scene::Renderstate const& rs, Filter filter,
                                          Sampler& sampler, scene::Worker& worker) const final;

    float3 evaluate_radiance(float3_p wi, float3_p uvw, float extent, Filter filter,
                             scene::Worker& worker) const final;

    float3 average_radiance(float area) const final;
};

class Sun_baked_material : public Material {
  public:
    Sun_baked_material(Sky* sky);

    scene::material::Sample const& sample(float3_p wo, scene::Ray const& ray,
                                          scene::Renderstate const& rs, Filter filter,
                                          Sampler& sampler, scene::Worker& worker) const final;

    float3 evaluate_radiance(float3_p wi, float3_p uvw, float extent, Filter filter,
                             scene::Worker& worker) const final;

    float3 average_radiance(float area) const final;

    void prepare_sampling(const Shape& shape, uint32_t part, Transformation const& trafo,
                          float area, bool importance_sampling, Threads& threads,
                          scene::Scene const& scene) final;

  private:
    math::Interpolated_function_1D<float3> emission_;
};

}  // namespace procedural::sky

#endif
