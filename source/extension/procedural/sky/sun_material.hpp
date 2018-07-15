#ifndef SU_EXTENSION_PROCEDURAL_SKY_SUN_MATERIAL_HPP
#define SU_EXTENSION_PROCEDURAL_SKY_SUN_MATERIAL_HPP

#include "base/math/function/interpolated_function.hpp"
#include "sky_material_base.hpp"

namespace procedural::sky {

class Sun_material : public Material {
  public:
    Sun_material(Sky& sky);

    virtual const scene::material::Sample& sample(f_float3 wo, scene::Renderstate const& rs,
                                                  Sampler_filter filter, sampler::Sampler& sampler,
                                                  scene::Worker const& worker,
                                                  uint32_t             depth) const override final;

    virtual float3 evaluate_radiance(f_float3 wi, float2 uv, float area, float time,
                                     Sampler_filter       filter,
                                     const scene::Worker& worker) const override final;

    virtual float3 average_radiance(float area) const override final;

    virtual void prepare_sampling(const Shape& shape, uint32_t part,
                                  Transformation const& transformation, float area,
                                  bool importance_sampling, thread::Pool& pool) override final;

    virtual size_t num_bytes() const override final;
};

class Sun_baked_material : public Material {
  public:
    Sun_baked_material(Sky& sky);

    virtual const scene::material::Sample& sample(f_float3 wo, scene::Renderstate const& rs,
                                                  Sampler_filter filter, sampler::Sampler& sampler,
                                                  scene::Worker const& worker,
                                                  uint32_t             depth) const override final;

    virtual float3 evaluate_radiance(f_float3 wi, float2 uv, float area, float time,
                                     Sampler_filter       filter,
                                     const scene::Worker& worker) const override final;

    virtual float3 average_radiance(float area) const override final;

    virtual void prepare_sampling(const Shape& shape, uint32_t part,
                                  Transformation const& transformation, float area,
                                  bool importance_sampling, thread::Pool& pool) override final;

    virtual size_t num_bytes() const override final;

  private:
    math::Interpolated_function<float3> emission_;
};

}  // namespace procedural::sky

#endif
