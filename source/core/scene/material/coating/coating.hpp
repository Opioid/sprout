#ifndef SU_CORE_SCENE_MATERIAL_COATING_COATING_HPP
#define SU_CORE_SCENE_MATERIAL_COATING_COATING_HPP

#include "scene/material/material_sample.hpp"

namespace scene::material::coating {

struct Result {
    float3 reflection;
    float3 attenuation;
    float  f;
    float  pdf;
};

class Clearcoat {
  public:
    using Sampler = sampler::Sampler;

    void set(float3_p absorption_coef, float thickness, float ior, float f0, float alpha,
             float weight);

    float3 attenuation(float n_dot_wo) const;

    float3 attenuation(float n_dot_wi, float n_dot_wo) const;

  protected:
    Result evaluate(float3_p wi, float3_p wo, float3_p h, float wo_dot_h, Layer const& layer,
                    bool avoid_caustics) const;

    Result evaluate_b(float3_p wi, float3_p wo, float3_p h, float wo_dot_h, Layer const& layer,
                      bool avoid_caustics) const;

    void reflect(float3_p wo, float3_p h, float n_dot_wo, float n_dot_h, float wi_dot_h,
                 float wo_dot_h, Layer const& layer, float3& attenuation,
                 bxdf::Sample& result) const;

    float sample(float3_p wo, Layer const& layer, Sampler& sampler, RNG& rng, float& n_dot_h,
                 bxdf::Sample& result) const;

  public:
    float3 absorption_coef_;

    float thickness_;
    float ior_;
    float f0_;
    float alpha_;
    float weight_;
};

class Thinfilm {
  public:
    using Sampler = sampler::Sampler;

    void set(float ior, float ior_internal, float alpha, float thickness);

    float3 attenuation(float n_dot_wo) const;

  protected:
    Result evaluate(float3_p wi, float3_p wo, float3_p h, float wo_dot_h, Layer const& layer,
                    bool avoid_caustics) const;

    Result evaluate_b(float3_p wi, float3_p wo, float3_p h, float wo_dot_h, Layer const& layer,
                      bool avoid_caustics) const;

    void reflect(float3_p wo, float3_p h, float n_dot_wo, float n_dot_h, float wi_dot_h,
                 float wo_dot_h, Layer const& layer, float3& attenuation,
                 bxdf::Sample& result) const;

    float sample(float3_p wo, Layer const& layer, Sampler& sampler, RNG& rng, float& n_dot_h,
                 bxdf::Sample& result) const;

  public:
    float ior_;
    float ior_internal_;
    float alpha_;
    float thickness_;
};

template <typename Coating>
class Coating_layer : public Layer, public Coating {
  public:
    using Sampler = sampler::Sampler;

    Result evaluate(float3_p wi, float3_p wo, float3_p h, float wo_dot_h,
                    bool avoid_caustics) const;

    Result evaluate_b(float3_p wi, float3_p wo, float3_p h, float wo_dot_h,
                      bool avoid_caustics) const;

    void reflect(float3_p wo, float3_p h, float n_dot_wo, float n_dot_h, float wi_dot_h,
                 float wo_dot_h, float3& attenuation, bxdf::Sample& result) const;

    float sample(float3_p wo, Sampler& sampler, RNG& rng, float& n_dot_h,
                 bxdf::Sample& result) const;
};

using Clearcoat_layer = Coating_layer<Clearcoat>;
using Thinfilm_layer  = Coating_layer<Thinfilm>;

}  // namespace scene::material::coating

#endif
