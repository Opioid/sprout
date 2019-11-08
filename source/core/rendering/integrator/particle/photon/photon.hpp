#ifndef SU_RENDERING_INTEGRATOR_PARTICLE_PHOTON_HPP
#define SU_RENDERING_INTEGRATOR_PARTICLE_PHOTON_HPP

#include "base/flags/flags.hpp"
#include "base/math/vector3.hpp"

namespace rendering::integrator::particle::photon {

struct Photon {
    float3 p;
    float3 wi;
    float  alpha[3];

    enum class Property { First_hit = 1 << 0, Volumetric = 1 << 1 };

    flags::Flags<Property> properties;
};

}  // namespace rendering::integrator::particle::photon

#endif
