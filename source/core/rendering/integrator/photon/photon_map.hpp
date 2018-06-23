#ifndef SU_RENDERING_INTEGRATOR_PHOTON_MAP_HPP
#define SU_RENDERING_INTEGRATOR_PHOTON_MAP_HPP

#include "base/math/vector3.hpp"

namespace scene::material {
class Sample;
}

namespace rendering::integrator::photon {

struct Photon {
    float3 p;
    float3 wi;
    float3 alpha;
};

class Map {
  public:
    Map(uint32_t num_photons);
    ~Map();

    void insert(Photon const& photon, int32_t index);

    float3 li(f_float3 position, scene::material::Sample const& sample) const;

  private:
    uint32_t num_photons_;
    Photon*  photons_;
};

}  // namespace rendering::integrator::photon

#endif
