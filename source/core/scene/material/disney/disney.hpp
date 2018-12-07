#ifndef SU_CORE_SCENE_MATERIAL_DISNEY_HPP
#define SU_CORE_SCENE_MATERIAL_DISNEY_HPP

#include "base/math/vector3.hpp"

namespace sampler {
class Sampler;
}

namespace scene ::material {

namespace bxdf {
struct Result;
struct Sample;
}  // namespace bxdf

class Sample;
struct Layer;

namespace disney {

using Sampler = sampler::Sampler;

class Isotropic {
  public:
    static bxdf::Result reflection(float h_dot_wi, float n_dot_wi, float n_dot_wo, float alpha,
                                   float3 const& color) noexcept;

    static float reflect(float3 const& wo, float n_dot_wo, Layer const& layer, float alpha,
                         float3 const& color, Sampler& sampler, bxdf::Sample& result) noexcept;

  private:
    static float3 evaluate(float h_dot_wi, float n_dot_wi, float n_dot_wo, float alpha,
                           float3 const& color) noexcept;
};

class Isotropic_no_lambert {
  public:
    static bxdf::Result reflection(float h_dot_wi, float n_dot_wi, float n_dot_wo, float alpha,
                                   float3 const& color) noexcept;

    static float reflect(float3 const& wo, float n_dot_wo, Layer const& layer, float alpha,
                         float3 const& color, Sampler& sampler, bxdf::Sample& result) noexcept;

  private:
    static float3 evaluate(float h_dot_wi, float n_dot_wi, float n_dot_wo, float alpha,
                           float3 const& color) noexcept;
};

}  // namespace disney
}  // namespace scene::material

#endif
