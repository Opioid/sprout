#ifndef SU_CORE_SCENE_MATERIAL_COLLISION_COEFFICIENTS_INL
#define SU_CORE_SCENE_MATERIAL_COLLISION_COEFFICIENTS_INL

#include "base/math/vector3.inl"
#include "collision_coefficients.hpp"

namespace scene::material {

inline CM::CM(float x) : minorant_mu_a(x), minorant_mu_s(x), minorant_mu_t(x), majorant_mu_t(x) {}

inline CM::CM(CC const& cc) {
    minorant_mu_a = /*0.5f **/ math::min_component(cc.a);
    minorant_mu_s = /*0.5f **/ math::min_component(cc.s);
    minorant_mu_t = minorant_mu_a + minorant_mu_s;
    majorant_mu_t = math::max_component(cc.a + cc.s);
}

}  // namespace scene::material

#endif
