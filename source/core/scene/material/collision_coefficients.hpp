#ifndef SU_CORE_SCENE_MATERIAL_COLLISION_COEFFICIENTS_HPP
#define SU_CORE_SCENE_MATERIAL_COLLISION_COEFFICIENTS_HPP

#include "base/math/vector3.hpp"

namespace scene::material {

struct CC {
    float3 a;
    float3 s;
};

struct alignas(16) CM {
    CM() = default;
    CM(float x);
    CM(CC const& cc);

    float minorant_mu_a;
    float minorant_mu_s;
    float minorant_mu_t;
    float majorant_mu_t;
};

}  // namespace scene::material

#endif
