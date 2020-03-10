#ifndef SU_CORE_SCENE_MATERIAL_COLLISION_COEFFICIENTS_INL
#define SU_CORE_SCENE_MATERIAL_COLLISION_COEFFICIENTS_INL

#include "base/math/vector3.inl"
#include "collision_coefficients.hpp"

namespace scene::material {

static inline float van_de_hulst(float g, float gs) {
    return (1.f - g) / (1 - gs);
}

static inline CC constexpr operator*(float s, CC const& cc) {
    return {s * cc.a, s * cc.s};
}

inline CM::CM() = default;

inline CM::CM(float x) : minorant_mu_a(x), minorant_mu_s(x), majorant_mu_a(x), majorant_mu_s(x) {}

inline CM::CM(float min, float max)
    : minorant_mu_a(min), minorant_mu_s(min), majorant_mu_a(max), majorant_mu_s(max) {}

inline CM::CM(CC const& cc) {
    minorant_mu_a = min_component(cc.a);
    minorant_mu_s = min_component(cc.s);
    majorant_mu_a = max_component(cc.a);
    majorant_mu_s = max_component(cc.s);
}

inline float CM::minorant_mu_t() const {
    return minorant_mu_a + minorant_mu_s;
}

inline float CM::majorant_mu_t() const {
    return majorant_mu_a + majorant_mu_s;
}

inline bool CM::is_empty() const {
    return 0.f == majorant_mu_a && 0.f == majorant_mu_s;
}

inline void CM::add(CC const& cc) {
    minorant_mu_a = std::min(minorant_mu_a, min_component(cc.a));
    minorant_mu_s = std::min(minorant_mu_s, min_component(cc.s));
    majorant_mu_a = std::max(majorant_mu_a, max_component(cc.a));
    majorant_mu_s = std::max(majorant_mu_s, max_component(cc.s));
}

static inline float3 attenuation_coefficient(float3 const& color, float distance) {
    float3 const ca = clamp(color, 0.001f, 0.99f);

    float3 const a = log(ca);

    return -a / distance;
}

static inline CC attenuation(float3 const& ac, float3 const& ssc, float distance) {
    float3 const mu_t = attenuation_coefficient(ac, distance);

    float3 const root = sqrt(9.59217f + 41.6808f * ssc + 17.7126f * ssc * ssc);

    float3 const factor = 4.09712f + 4.20863f * ssc - root;

    float3 const pss = 1.f - (factor * factor);

    float3 const mu_a = mu_t * (1.f - pss);

    return {mu_a, mu_t - mu_a};
}

}  // namespace scene::material

#endif
