#pragma once

#include "scene/material/material_sample.hpp"

namespace scene::material::substitute {

template <typename Diffuse, class... Layer_data>
class Sample_base : public material::Sample {
  public:
    Layer const& base_layer() const override final;

    float3 radiance() const override final;

  protected:
    template <typename Coating>
    bxdf::Result base_and_coating_evaluate(float3 const& wi, Coating const& coating_layer) const;

    template <typename Coating>
    void base_and_coating_sample(Coating const& coating_layer, sampler::Sampler& sampler,
                                 bxdf::Sample& result) const;

    template <typename Coating>
    void diffuse_sample_and_coating(Coating const& coating_layer, sampler::Sampler& sampler,
                                    bxdf::Sample& result) const;

    template <typename Coating>
    void specular_sample_and_coating(Coating const& coating_layer, sampler::Sampler& sampler,
                                     bxdf::Sample& result) const;

    template <typename Coating>
    void pure_specular_sample_and_coating(Coating const& coating_layer, sampler::Sampler& sampler,
                                          bxdf::Sample& result) const;

  public:
    struct Layer : material::Sample::Layer, Layer_data... {
        void set(float3 const& color, float3 const& radiance, float ior, float constant_f0,
                 float roughness, float metallic);

        bxdf::Result base_evaluate(float3 const& wi, float3 const& wo, float3 const& h,
                                   float wo_dot_h, bool avoid_caustics) const;

        void diffuse_sample(float3 const& wo, sampler::Sampler& sampler, bool avoid_caustics,
                            bxdf::Sample& result) const;

        void specular_sample(float3 const& wo, sampler::Sampler& sampler,
                             bxdf::Sample& result) const;

        void pure_specular_sample(float3 const& wo, sampler::Sampler& sampler,
                                  bxdf::Sample& result) const;

        float base_diffuse_fresnel_hack(float n_dot_wi, float n_dot_wo) const;

        float3 diffuse_color_;
        float3 f0_;
        float3 emission_;

        float ior_;
        float roughness_;
        float alpha_;
        float alpha2_;
        float metallic_;
    };

    Layer layer_;

    bool avoid_caustics_;
};

}  // namespace scene::material::substitute
