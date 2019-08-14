#ifndef SU_EXTENSION_PROCEDURAL_SKY_SUN_MATERIAL_HPP
#define SU_EXTENSION_PROCEDURAL_SKY_SUN_MATERIAL_HPP

#include "base/math/interpolated_function_1d.hpp"
#include "sky_material_base.hpp"

namespace procedural::sky {

class Sun_material : public Material {
  public:
    Sun_material(Sky& sky) noexcept;

    scene::material::Sample const& sample(float3 const& wo, scene::Ray const& ray,
                                          scene::Renderstate const& rs, Filter filter,
                                          Sampler& sampler, scene::Worker const& worker) const
        noexcept override final;

    float3 evaluate_radiance(float3 const& wi, float2 uv, float area, Filter filter,
                             scene::Worker const& worker) const noexcept override final;

    float3 average_radiance(float area) const noexcept override final;

    void prepare_sampling(const Shape& shape, uint32_t part, uint64_t time,
                          Transformation const& transformation, float area,
                          bool importance_sampling, thread::Pool& pool) noexcept override final;

    size_t num_bytes() const noexcept override final;
};

class Sun_baked_material : public Material {
  public:
    Sun_baked_material(Sky& sky) noexcept;

    scene::material::Sample const& sample(float3 const& wo, scene::Ray const& ray,
                                          scene::Renderstate const& rs, Filter filter,
                                          Sampler& sampler, scene::Worker const& worker) const
        noexcept override final;

    float3 evaluate_radiance(float3 const& wi, float2 uv, float area, Filter filter,
                             scene::Worker const& worker) const noexcept override final;

    float3 average_radiance(float area) const noexcept override final;

    void prepare_sampling(const Shape& shape, uint32_t part, uint64_t time,
                          Transformation const& transformation, float area,
                          bool importance_sampling, thread::Pool& pool) noexcept override final;

    size_t num_bytes() const noexcept override final;

  private:
    math::Interpolated_function_1D<float3> emission_;
};

}  // namespace procedural::sky

#endif
