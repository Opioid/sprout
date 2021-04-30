#ifndef SU_EXTENSION_PROCEDURAL_SKY_SKY_MATERIAL_HPP
#define SU_EXTENSION_PROCEDURAL_SKY_SKY_MATERIAL_HPP

#include "base/math/distribution/distribution_2d.hpp"
#include "core/image/texture/texture.hpp"
#include "core/image/typed_image.hpp"
#include "sky_material_base.hpp"

namespace resource {
class Manager;
}

namespace procedural::sky {

class Sky_material : public Material {
  public:
    Sky_material(Sky* sky);

    scene::material::Sample const& sample(float3_p wo, scene::Ray const& ray,
                                          scene::Renderstate const& rs, Filter filter,
                                          Sampler& sampler, scene::Worker& worker) const final;

    float3 evaluate_radiance(float3_p wi, float3_p uvw, float extent, Filter filter,
                             scene::Worker const& worker) const final;

    float3 average_radiance(float area) const final;

    void prepare_sampling(Shape const& shape, uint32_t part, Transformation const& trafo,
                          float area, bool importance_sampling, Threads& threads,
                          scene::Scene const& scene) final;
};

class Sky_baked_material : public Material {
  public:
    using Resources = resource::Manager;

    Sky_baked_material(Sky* sky, Resources& resources);

    ~Sky_baked_material() override;

    scene::material::Sample const& sample(float3_p wo, scene::Ray const& ray,
                                          scene::Renderstate const& rs, Filter filter,
                                          Sampler& sampler, scene::Worker& worker) const final;

    float3 evaluate_radiance(float3_p wi, float3_p uvw, float extent, Filter filter,
                             scene::Worker const& worker) const final;

    float3 average_radiance(float area) const final;

    Radiance_sample radiance_sample(float3_p r3) const final;

    float emission_pdf(float3_p uvw, Filter filter, scene::Worker const& worker) const final;

    void prepare_sampling(const Shape& shape, uint32_t part, Transformation const& trafo,
                          float area, bool importance_sampling, Threads& threads,
                          scene::Scene const& scene) final;

  private:
    static float3 unclipped_canopy_mapping(Transformation const& trafo, float2 uv);

    image::Image*           cache_;
    image::texture::Texture texture_;

    float3 average_emission_;

    float total_weight_;

    math::Distribution_2D distribution_;
};

}  // namespace procedural::sky

#endif
