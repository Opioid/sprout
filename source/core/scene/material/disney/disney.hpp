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

namespace disney {

class Isotropic {
  public:
    template <typename Layer>
    static bxdf::Result reflection(float h_dot_wi, float n_dot_wi, float n_dot_wo,
                                   Layer const& layer) noexcept;

    template <typename Layer>
    static float reflect(float3 const& wo, float n_dot_wo, Layer const& layer,
                         sampler::Sampler& sampler, bxdf::Sample& result) noexcept;

  private:
    template <typename Layer>
    static float3 evaluate(float h_dot_wi, float n_dot_wi, float n_dot_wo, Layer const& layer) noexcept;
};

class Isotropic_no_lambert {
  public:
    template <typename Layer>
    static bxdf::Result reflection(float h_dot_wi, float n_dot_wi, float n_dot_wo,
                                   Layer const& layer) noexcept;

    template <typename Layer>
    static float reflect(float3 const& wo, float n_dot_wo, Layer const& layer,
                         sampler::Sampler& sampler, bxdf::Sample& result) noexcept;

  private:
    template <typename Layer>
    static float3 evaluate(float h_dot_wi, float n_dot_wi, float n_dot_wo, Layer const& layer) noexcept;
};

}  // namespace disney
}  // namespace scene::material

#endif
