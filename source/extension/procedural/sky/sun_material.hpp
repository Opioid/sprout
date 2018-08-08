#ifndef SU_EXTENSION_PROCEDURAL_SKY_SUN_MATERIAL_HPP
#define SU_EXTENSION_PROCEDURAL_SKY_SUN_MATERIAL_HPP

#include "base/math/function/interpolated_function.hpp"
#include "sky_material_base.hpp"

namespace procedural::sky {

class Sun_material : public Material {
  public:
    Sun_material(Sky& sky) noexcept;

    const scene::material::Sample& sample(float3 const& wo, scene::Renderstate const& rs,
                                          Sampler_filter filter, sampler::Sampler& sampler,
                                          scene::Worker const& worker, uint32_t depth) const
        noexcept override final;

    float3 evaluate_radiance(float3 const& wi, float2 uv, float area, float time,
                             Sampler_filter filter, const scene::Worker& worker) const
        noexcept override final;

    float3 average_radiance(float area) const noexcept override final;

    void prepare_sampling(const Shape& shape, uint32_t part, Transformation const& transformation,
                          float area, bool importance_sampling,
                          thread::Pool& pool) noexcept override final;

    size_t num_bytes() const noexcept override final;
};

class Sun_baked_material : public Material {
  public:
    Sun_baked_material(Sky& sky) noexcept;

    const scene::material::Sample& sample(float3 const& wo, scene::Renderstate const& rs,
                                          Sampler_filter filter, sampler::Sampler& sampler,
                                          scene::Worker const& worker, uint32_t depth) const
        noexcept override final;

    float3 evaluate_radiance(float3 const& wi, float2 uv, float area, float time,
                             Sampler_filter filter, const scene::Worker& worker) const
        noexcept override final;

    float3 average_radiance(float area) const noexcept override final;

    void prepare_sampling(const Shape& shape, uint32_t part, Transformation const& transformation,
                          float area, bool importance_sampling,
                          thread::Pool& pool) noexcept override final;

    size_t num_bytes() const noexcept override final;

  private:
    math::Interpolated_function<float3> emission_;
};

}  // namespace procedural::sky

#endif
