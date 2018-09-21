#pragma once

#include "base/math/vector3.hpp"

namespace sampler {
class Sampler;
}

namespace scene::material {

namespace bxdf {
struct Result;
struct Sample;
}  // namespace bxdf

class Sample;
struct Layer;

namespace lambert {

class Isotropic {
  public:
    static bxdf::Result reflection(float3 const& color, float n_dot_wi,
                                   Layer const& layer) noexcept;

    static float reflect(float3 const& color, Layer const& layer, sampler::Sampler& sampler,
                         bxdf::Sample& result) noexcept;
};

}  // namespace lambert
}  // namespace scene::material
