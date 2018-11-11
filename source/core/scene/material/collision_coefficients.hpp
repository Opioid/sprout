#ifndef SU_CORE_SCENE_MATERIAL_COLLISION_COEFFICIENTS_HPP
#define SU_CORE_SCENE_MATERIAL_COLLISION_COEFFICIENTS_HPP

#include "base/math/vector3.hpp"

namespace scene::material {

struct CC {
    float3 a;
    float3 s;
};

struct alignas(16) CM {
    CM() noexcept = default;
    CM(float x) noexcept;
    CM(CC const& cc) noexcept;

    float minorant_mu_t() const noexcept;
    float majorant_mu_t() const noexcept;

    float minorant_mu_a;
    float minorant_mu_s;
    float majorant_mu_a;
    float majorant_mu_s;
};

}  // namespace scene::material

#endif
