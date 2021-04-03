#ifndef SU_CORE_SCENE_MATERIAL_BXDF_HPP
#define SU_CORE_SCENE_MATERIAL_BXDF_HPP

#include "base/flags/flags.hpp"
#include "base/math/vector3.hpp"

namespace scene::material::bxdf {

enum class Type {
    Reflection   = 1 << 0,
    Transmission = 1 << 1,
    Diffuse      = 1 << 2,
    Glossy       = 1 << 3,
    Specular     = 1 << 4,
    Straight     = 1 << 5,

    Diffuse_reflection    = Reflection | Diffuse,
    Glossy_reflection     = Reflection | Glossy,
    Specular_reflection   = Reflection | Specular,
    Diffuse_transmission  = Transmission | Diffuse,
    Glossy_transmission   = Transmission | Glossy,
    Specular_transmission = Transmission | Specular,
    Straight_transmission = Transmission | Straight
};

struct Result {
    inline Result(float3_p reflection, float pdf)
        : reflection(reflection[0], reflection[1], reflection[2], pdf) {}

    float3 reflection;

    inline float pdf() const {
        return reflection[3];
    }

    inline float& pdf() {
        return reflection[3];
    }
};

struct Sample {
    float3 reflection;
    float3 wi;
    float3 h;  // intermediate result, convenient to store here

    float pdf = 0.f;
    float wavelength;
    float h_dot_wi;  // intermediate result, convenient to store here

    using Type_flag = flags::Flags<Type>;
    Type_flag type;
};

}  // namespace scene::material::bxdf

#endif
