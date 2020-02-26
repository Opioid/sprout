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

static inline float3 extinction_coefficient(float3 const& color, float distance) {
    float3 const ca = clamp(color, 0.001f, 0.99f);

    float3 const a = log(ca);

    return -a / distance;
}

static inline CC attenuation(float3 const& ac, float3 const& ssc, float distance) {
    float3 const mu_t = extinction_coefficient(ac, distance);

    float3 const root = sqrt(9.59217f + 41.6808f * ssc + 17.7126f * ssc * ssc);

    float3 const factor = 4.09712f + 4.20863f * ssc - root;

    float3 const pss = 1.f - (factor * factor);

    float3 const mu_a = mu_t * (1.f - pss);

    return {mu_a, mu_t - mu_a};
}

static inline CC attenuation(float3 const& color, float distance) {
    return attenuation(color, color, distance);
}

// Supposedly this:
// https://disney-animation.s3.amazonaws.com/uploads/production/publication_asset/153/asset/siggraph2016SSS.pdf
// But looks wrong (Well, the emily head looks better with the above)?!

static inline CC disney_attenuation(float3 const& color, float distance) {
    float3 const a  = color;
    float3 const a2 = a * a;
    float3 const a3 = a2 * a;

    float3 const alpha = float3(1.f) - exp(-5.09406f * a + 2.61188f * a2 - 4.31805f * a3);

    float3 const s = float3(1.9f) - a + 3.5f * (a - float3(0.8f)) * (a - float3(0.8f));

    float3 const mu_t = 1.0f / (distance * s);

    float3 const mu_s = alpha * mu_t;

    return {mu_t - mu_s, mu_s};
}

}  // namespace scene::material

#endif
