#ifndef SU_CORE_SCENE_MATERIAL_LAMBERT_HPP
#define SU_CORE_SCENE_MATERIAL_LAMBERT_HPP

#include "base/math/vector.hpp"

namespace sampler {
class Sampler;
}

namespace rnd {
class Generator;
}

using RNG = rnd::Generator;

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
    static bxdf::Result reflection(float3_p color, float n_dot_wi, Layer const& layer);

    static float reflect(float3_p color, Layer const& layer, sampler::Sampler& sampler, RNG& rng,
                         bxdf::Sample& result);
};

}  // namespace lambert
}  // namespace scene::material

#endif
