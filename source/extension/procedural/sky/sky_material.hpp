#ifndef SU_EXTENSION_PROCEDURAL_SKY_SKY_MATERIAL_HPP
#define SU_EXTENSION_PROCEDURAL_SKY_SKY_MATERIAL_HPP

#include "base/math/distribution/distribution_2d.hpp"
#include "sky_material_base.hpp"

namespace procedural::sky {

class Sky_material : public Material {
  public:
    Sky_material(Sky& sky) noexcept;

    const scene::material::Sample& sample(float3 const& wo, scene::Ray const& ray,
                                          const scene::Renderstate& rs, Filter filter,
                                          sampler::Sampler& sampler, const scene::Worker& worker,
                                          uint32_t depth) const noexcept override final;

    float3 evaluate_radiance(float3 const& wi, float2 uv, float area, Filter filter,
                             const scene::Worker& worker) const noexcept override final;

    float3 average_radiance(float area) const noexcept override final;

    void prepare_sampling(const Shape& shape, uint32_t part, uint64_t time,
                          Transformation const& transformation, float area,
                          bool importance_sampling, thread::Pool& pool) noexcept override final;

    size_t num_bytes() const noexcept override final;
};

class Sky_baked_material : public Material {
  public:
    Sky_baked_material(Sky& sky) noexcept;

    ~Sky_baked_material() noexcept override;

    const scene::material::Sample& sample(float3 const& wo, scene::Ray const& ray,
                                          const scene::Renderstate& rs, Filter filter,
                                          sampler::Sampler& sampler, const scene::Worker& worker,
                                          uint32_t depth) const noexcept override final;

    float3 evaluate_radiance(float3 const& wi, float2 uv, float area, Filter filter,
                             const scene::Worker& worker) const noexcept override final;

    float3 average_radiance(float area) const noexcept override final;

    bool has_emission_map() const noexcept override final;

    Sample_2D radiance_sample(float2 r2) const noexcept override final;

    float emission_pdf(float2 uv, Filter filter, const scene::Worker& worker) const
        noexcept override final;

    void prepare_sampling(const Shape& shape, uint32_t part, uint64_t time,
                          Transformation const& transformation, float area,
                          bool importance_sampling, thread::Pool& pool) noexcept override final;

    size_t num_bytes() const noexcept override final;

  private:
    static float3 unclipped_canopy_mapping(Transformation const& transformation,
                                           float2                uv) noexcept;

    Texture_adapter emission_map_;

    float3 average_emission_;

    float total_weight_;

    math::Distribution_2D distribution_;
};

}  // namespace procedural::sky

#endif
