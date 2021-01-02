#ifndef SU_CORE_SCENE_MATERIAL_GGX_HPP
#define SU_CORE_SCENE_MATERIAL_GGX_HPP

#include "base/math/vector3.hpp"

namespace scene::material {

class Sample;
struct Layer;
struct IoR;

namespace bxdf {
struct Result;
struct Sample;
}  // namespace bxdf

namespace ggx {

class Iso {
  public:
    template <typename Fresnel>
    static bxdf::Result reflection(float n_dot_wi, float n_dot_wo, float wo_dot_h, float n_dot_h,
                                   float alpha, Fresnel fresnel);

    template <typename Fresnel>
    static bxdf::Result reflection(float n_dot_wi, float n_dot_wo, float wo_dot_h, float n_dot_h,
                                   float alpha, Fresnel fresnel, float3& fresnel_result);

    template <typename Fresnel>
    static float reflect(float3_p wo, float n_dot_wo, float alpha, Fresnel fresnel, float2 xi,
                         Layer const& layer, bxdf::Sample& result);

    template <typename Fresnel>
    static float reflect(float3_p wo, float n_dot_wo, float alpha, Fresnel fresnel, float2 xi,
                         Layer const& layer, float3& fresnel_result, bxdf::Sample& result);

    template <typename Fresnel>
    static bxdf::Result refraction(float n_dot_wi, float n_dot_wo, float wi_dot_h, float wo_dot_h,
                                   float n_dot_h, float alpha, IoR ior, Fresnel fresnel);

    template <typename Fresnel>
    static float refract(float3_p wo, float n_dot_wo, float alpha, IoR ior, Fresnel fresnel,
                         float2 xi, Layer const& layer, bxdf::Sample& result);

    static float3 sample(float3_p wo, float alpha, float2 xi, Layer const& layer, float& n_dot_h);

    static float reflect(float3_p wo, float3_p h, float n_dot_wo, float n_dot_h, float wi_dot_h,
                         float wo_dot_h, float alpha, Layer const& layer, bxdf::Sample& result);

    static float refract(float3_p wo, float3_p h, float n_dot_wo, float n_dot_h, float wi_dot_h,
                         float wo_dot_h, float alpha, IoR ior, Layer const& layer,
                         bxdf::Sample& result);
};

class Aniso {
  public:
    template <typename Fresnel>
    static bxdf::Result reflection(float3_p h, float n_dot_wi, float n_dot_wo, float wo_dot_h,
                                   float2 alpha, Layer const& layer, Fresnel fresnel);

    template <typename Fresnel>
    static float reflect(float3_p wo, float n_dot_wo, float2 alpha, Layer const& layer,
                         Fresnel fresnel, float2 xi, bxdf::Sample& result);
};

}  // namespace ggx
}  // namespace scene::material

#endif
