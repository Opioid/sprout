#pragma once

#include "scene/material/material_sample.hpp"

namespace scene::material::substitute {

template <typename Diffuse, class... Layer_data>
class Sample_base : public material::Sample {
  public:
    virtual Layer const& base_layer() const override final;

    virtual float3 radiance() const override final;

  protected:
    template <typename Coating>
    bxdf::Result base_and_coating_evaluate(f_float3 wi, const Coating& coating_layer) const;

    template <typename Coating>
    void base_and_coating_sample(const Coating& coating_layer, sampler::Sampler& sampler, bxdf::Sample& result) const;

    template <typename Coating>
    void diffuse_sample_and_coating(const Coating& coating_layer, sampler::Sampler& sampler, bxdf::Sample& result) const;

    template <typename Coating>
    void specular_sample_and_coating(const Coating& coating_layer, sampler::Sampler& sampler,
                                     bxdf::Sample& result) const;

    template <typename Coating>
    void pure_specular_sample_and_coating(const Coating& coating_layer, sampler::Sampler& sampler,
                                          bxdf::Sample& result) const;

  public:
    struct Layer : material::Sample::Layer, Layer_data... {
        void set(f_float3 color, f_float3 radiance, float ior, float constant_f0, float roughness,
                 float metallic);

        bxdf::Result base_evaluate(f_float3 wi, f_float3 wo, f_float3 h, float wo_dot_h,
                                   bool avoid_caustics) const;

        void diffuse_sample(f_float3 wo, sampler::Sampler& sampler, bool avoid_caustics,
                            bxdf::Sample& result) const;

        void specular_sample(f_float3 wo, sampler::Sampler& sampler, bxdf::Sample& result) const;

        void pure_specular_sample(f_float3 wo, sampler::Sampler& sampler,
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
