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

class Isotropic {
  public:
    template <typename Fresnel>
    static bxdf::Result reflection(float n_dot_wi, float n_dot_wo, float wo_dot_h, float n_dot_h,
                                   float alpha, Fresnel const& fresnel);

    template <typename Fresnel>
    static bxdf::Result reflection(float n_dot_wi, float n_dot_wo, float wo_dot_h, float n_dot_h,
                                   float alpha, Fresnel const& fresnel, float3& fresnel_result);

    template <typename Fresnel>
    static float reflect(float3_p wo, float n_dot_wo, Layer const& layer, float alpha,
                         Fresnel const& fresnel, float2 xi, bxdf::Sample& result);

    template <typename Fresnel>
    static float reflect(float3_p wo, float n_dot_wo, Layer const& layer, float alpha,
                         Fresnel const& fresnel, float2 xi, float3& fresnel_result,
                         bxdf::Sample& result);

    template <typename Fresnel>
    static bxdf::Result refraction(float n_dot_wi, float n_dot_wo, float wi_dot_h, float wo_dot_h,
                                   float n_dot_h, float alpha, IoR const& ior,
                                   Fresnel const& fresnel);

    template <typename Fresnel>
    static float refract(float3_p wo, float n_dot_wo, Layer const& layer, float alpha,
                         IoR const& ior, Fresnel const& fresnel, float2 xi, bxdf::Sample& result);

    static float3 sample(float3_p wo, Layer const& layer, float alpha, float2 xi, float& n_dot_h);

    static float reflect(float3_p wo, float3_p h, float n_dot_wo, float n_dot_h, float wi_dot_h,
                         float wo_dot_h, Layer const& layer, float alpha, bxdf::Sample& result);

    static float refract(float3_p wo, float3_p h, float n_dot_wo, float n_dot_h, float wi_dot_h,
                         float wo_dot_h, Layer const& layer, float alpha, IoR const& ior,
                         bxdf::Sample& result);
};

class Anisotropic {
  public:
    template <typename Fresnel>
    static bxdf::Result reflection(float3_p h, float n_dot_wi, float n_dot_wo, float wo_dot_h,
                                   float2 alpha, Layer const& layer, Fresnel const& fresnel);

    template <typename Fresnel>
    static float reflect(float3_p wo, float n_dot_wo, float2 alpha, Layer const& layer,
                         Fresnel const& fresnel, float2 xi, bxdf::Sample& result);
};

}  // namespace ggx
}  // namespace scene::material

#endif
