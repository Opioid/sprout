#ifndef SU_CORE_SCENE_MATERIAL_COLLISION_COEFFICIENTS_HPP
#define SU_CORE_SCENE_MATERIAL_COLLISION_COEFFICIENTS_HPP

#include "base/math/vector3.hpp"

namespace scene::material {

struct CC {
    float3 a;
    float3 s;
};

struct CCE {
    CC     cc;
    float3 e;
};

struct alignas(16) CM {
    CM();
    CM(float x);
    CM(float min, float max);
    CM(CC const& cc);

    float minorant_mu_t() const;
    float majorant_mu_t() const;

    bool is_empty() const;

    void add(CC const& cc);

    float minorant_mu_a;
    float minorant_mu_s;
    float majorant_mu_a;
    float majorant_mu_s;
};

}  // namespace scene::material

#endif
