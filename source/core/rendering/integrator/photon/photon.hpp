#ifndef SU_RENDERING_INTEGRATOR_PHOTON_HPP
#define SU_RENDERING_INTEGRATOR_PHOTON_HPP

#include "base/math/vector3.hpp"
#include "base/flags/flags.hpp"

namespace rendering::integrator::photon {

struct Photon {
    float3 p;
    float3 wi;
    float  alpha[3];

    enum class Property { First_hit = 1 << 0, Volumetric = 1 << 1 };

    flags::Flags<Property> properties;
};

struct Photon_ref {
    int32_t id;
    float   sd;
};

}

#endif
