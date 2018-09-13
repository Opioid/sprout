#ifndef SU_CORE_SCENE_MATERIAL_COLLISION_COEFFICIENTS_INL
#define SU_CORE_SCENE_MATERIAL_COLLISION_COEFFICIENTS_INL

#include "base/math/vector3.inl"
#include "collision_coefficients.hpp"

namespace scene::material {

inline CM::CM(float x) noexcept
    : minorant_mu_a(x), minorant_mu_s(x), minorant_mu_t(x), majorant_mu_t(x) {}

inline CM::CM(CC const& cc) noexcept {
    minorant_mu_a = /*0.5f **/ math::min_component(cc.a);
    minorant_mu_s = /*0.5f **/ math::min_component(cc.s);
    minorant_mu_t = minorant_mu_a + minorant_mu_s;
    majorant_mu_t = math::max_component(cc.a + cc.s);
}

static inline float3 extinction_coefficient(float3 const& color, float distance) {
    float3 const ca = math::clamp(color, 0.001f, 0.99999f);

    float3 const a = math::log(ca);

    return -a / distance;
}

static inline CC attenuation(float3 const& ac, float3 const& ssc, float distance) {
    float3 const mu_t = extinction_coefficient(ac, distance);

    float3 const root = math::sqrt(9.59217f + 41.6898f * ssc + 17.71226f * ssc * ssc);

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
// But looks wrong?!

static inline CC disney_attenuation(float3 const& color, float distance) {
    float3 const a  = color;
    float3 const a2 = a * a;
    float3 const a3 = a2 * a;

    float3 const alpha = float3(1.0f) - math::exp(-5.09406f * a + 2.61188f * a2 - 4.31805f * a3);

    float3 const s = float3(1.9f) - a + 3.5f * (a - float3(0.8f)) * (a - float3(0.8f));

    float3 const mu_t = 1.0f / (distance * s);

    float3 const mu_s = alpha * mu_t;

    return {mu_t - mu_s, mu_s};
}

}  // namespace scene::material

#endif
