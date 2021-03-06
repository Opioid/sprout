#ifndef SU_CORE_SCENE_MATERIAL_DISNEY_HPP
#define SU_CORE_SCENE_MATERIAL_DISNEY_HPP

#include "base/math/vector3.hpp"

namespace scene::material {

namespace bxdf {
struct Result;
struct Sample;
}  // namespace bxdf

class Sample;
struct Layer;

namespace disney {

class Iso {
  public:
    static bxdf::Result reflection(float h_dot_wi, float n_dot_wi, float n_dot_wo, float alpha,
                                   float3_p color);

    static float reflect(float3_p wo, float n_dot_wo, Layer const& layer, float alpha,
                         float3_p color, float2 xi, bxdf::Sample& result);

  private:
    static float3 evaluate(float h_dot_wi, float n_dot_wi, float n_dot_wo, float alpha,
                           float3_p color);
};

class Isotropic_no_lambert {
  public:
    static bxdf::Result reflection(float h_dot_wi, float n_dot_wi, float n_dot_wo, float alpha,
                                   float3_p color);

    static float reflect(float3_p wo, float n_dot_wo, Layer const& layer, float alpha,
                         float3_p color, float2 xi, bxdf::Sample& result);

  private:
    static float3 evaluate(float h_dot_wi, float n_dot_wi, float n_dot_wo, float alpha,
                           float3_p color);
};

}  // namespace disney
}  // namespace scene::material

#endif
