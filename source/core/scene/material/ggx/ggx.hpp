#ifndef SU_CORE_SCENE_MATERIAL_GGX_HPP
#define SU_CORE_SCENE_MATERIAL_GGX_HPP

#include "base/math/vector3.hpp"

namespace sampler {
class Sampler;
}

namespace scene::material {

class Sample;

namespace bxdf {
struct Result;
struct Sample;
}  // namespace bxdf

namespace ggx {

class Isotropic {
  public:
    template <typename Layer, typename Fresnel>
    static bxdf::Result reflection(float n_dot_wi, float n_dot_wo, float wo_dot_h, float n_dot_h,
                                   Layer const& layer, Fresnel const& fresnel) noexcept;

    template <typename Layer, typename Fresnel>
    static bxdf::Result reflection(float n_dot_wi, float n_dot_wo, float wo_dot_h, float n_dot_h,
                                   Layer const& layer, Fresnel const& fresnel,
                                   float3& fresnel_result) noexcept;

    template <typename Layer, typename Fresnel>
    static float reflect(float3 const& wo, float n_dot_wo, Layer const& layer,
                         Fresnel const& fresnel, sampler::Sampler& sampler,
                         bxdf::Sample& result) noexcept;

    template <typename Layer, typename Fresnel>
    static float reflect(float3 const& wo, float n_dot_wo, Layer const& layer,
                         Fresnel const& fresnel, sampler::Sampler& sampler, float3& fresnel_result,
                         bxdf::Sample& result) noexcept;

    template <typename Layer, typename IOR, typename Fresnel>
    static float reflect_internally(float3 const& wo, float n_dot_wo, Layer const& layer,
                                    IOR const& ior, Fresnel const& fresnel,
                                    sampler::Sampler& sampler, bxdf::Sample& result) noexcept;

    template <typename Layer, typename IoR, typename Fresnel>
    static bxdf::Result refraction(float n_dot_wi, float n_dot_wo, float wi_dot_h, float wo_dot_h, float n_dot_h,
                                   Layer const& layer, IoR const& ior, Fresnel const& fresnel) noexcept;

    template <typename Layer, typename IoR, typename Fresnel>
    static float refract(float3 const& wo, float n_dot_wo, Layer const& layer, IoR const& ior,
                         Fresnel const& fresnel, sampler::Sampler& sampler,
                         bxdf::Sample& result) noexcept;
};

class Anisotropic {
  public:
    template <typename Layer, typename Fresnel>
    static bxdf::Result reflection(float3 const& h, float n_dot_wi, float n_dot_wo, float wo_dot_h,
                                   Layer const& layer, Fresnel const& fresnel) noexcept;

    template <typename Layer, typename Fresnel>
    static float reflect(float3 const& wo, float n_dot_wo, Layer const& layer,
                         Fresnel const& fresnel, sampler::Sampler& sampler,
                         bxdf::Sample& result) noexcept;
};

}  // namespace ggx
}  // namespace scene::material

#endif
