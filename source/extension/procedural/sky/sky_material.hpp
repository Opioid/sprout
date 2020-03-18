#ifndef SU_EXTENSION_PROCEDURAL_SKY_SKY_MATERIAL_HPP
#define SU_EXTENSION_PROCEDURAL_SKY_SKY_MATERIAL_HPP

#include "base/math/distribution/distribution_2d.hpp"
#include "core/image/texture/texture.hpp"
#include "core/image/texture/texture_float3.hpp"
#include "core/image/typed_image.hpp"
#include "sky_material_base.hpp"

namespace procedural::sky {

class Sky_material : public Material {
  public:
    Sky_material(Sky& sky);

    scene::material::Sample const& sample(float3 const& wo, scene::Ray const& ray,
                                          const scene::Renderstate& rs, Filter filter,
                                          Sampler& sampler, scene::Worker& worker) const final;

    float3 evaluate_radiance(float3 const& wi, float2 uv, float area, Filter filter,
                             scene::Worker const& worker) const final;

    float3 average_radiance(float area, scene::Scene const& scene) const final;

    void prepare_sampling(Shape const& shape, uint32_t part, uint64_t time,
                          Transformation const& transformation, float area,
                          bool importance_sampling, thread::Pool& threads,
                          scene::Scene const& scene) final;

    size_t num_bytes() const final;
};

class Sky_baked_material : public Material {
  public:
    Sky_baked_material(Sky& sky);

    ~Sky_baked_material() override;

    scene::material::Sample const& sample(float3 const& wo, scene::Ray const& ray,
                                          const scene::Renderstate& rs, Filter filter,
                                          Sampler& sampler, scene::Worker& worker) const final;

    float3 evaluate_radiance(float3 const& wi, float2 uv, float area, Filter filter,
                             scene::Worker const& worker) const final;

    float3 average_radiance(float area, scene::Scene const& scene) const final;

    Sample_2D radiance_sample(float2 r2) const final;

    float emission_pdf(float2 uv, Filter filter, scene::Worker const& worker) const final;

    void prepare_sampling(const Shape& shape, uint32_t part, uint64_t time,
                          Transformation const& transformation, float area,
                          bool importance_sampling, thread::Pool& threads,
                          scene::Scene const& scene) final;

    size_t num_bytes() const final;

  private:
    static float3 unclipped_canopy_mapping(Transformation const& transformation, float2 uv);

    image::Float3 cache_;

    image::texture::Texture cache_texture_;

    float3 average_emission_;

    float total_weight_;

    math::Distribution_2D distribution_;
};

}  // namespace procedural::sky

#endif
