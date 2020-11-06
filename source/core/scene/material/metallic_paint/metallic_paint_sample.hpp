#pragma once

#include "scene/material/coating/coating.hpp"
#include "scene/material/material_sample.hpp"

namespace scene::material::metallic_paint {

class Sample : public material::Sample {
  public:
    bxdf::Result evaluate_f(float3 const& wi) const final;

    bxdf::Result evaluate_b(float3 const& wi) const final;

    void sample(Sampler& sampler, RNG& rng, bxdf::Sample& result) const final;

    struct Base_layer : material::Layer {
        void set(float3 const& color_a, float3 const& color_b, float alpha);

        template <bool Forward>
        bxdf::Result evaluate(float3 const& wi, float3 const& wo, float3 const& h,
                              float wo_dot_h) const;

        void sample(float3 const& wo, Sampler& sampler, RNG& rng, bxdf::Sample& result) const;

        float3 color_a_;
        float3 color_b_;

        float alpha_;
    };

    struct Flakes_layer : material::Layer {
        void set(float3 const& ior, float3 const& absorption, float alpha, float weight);

        template <bool Forward>
        bxdf::Result evaluate(float3 const& wi, float3 const& wo, float3 const& h, float wo_dot_h,
                              float3& fresnel_result) const;

        void sample(float3 const& wo, Sampler& sampler, RNG& rng, float3& fresnel_result,
                    bxdf::Sample& result) const;

        float3 ior_;
        float3 absorption_;

        float alpha_;
        float weight_;
    };

    Base_layer base_;

    Flakes_layer flakes_;

    coating::Clearcoat_layer coating_;
};

}  // namespace scene::material::metallic_paint
