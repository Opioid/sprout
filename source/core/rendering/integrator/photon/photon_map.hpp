#ifndef SU_RENDERING_INTEGRATOR_PHOTON_MAP_HPP
#define SU_RENDERING_INTEGRATOR_PHOTON_MAP_HPP

#include "base/math/aabb.hpp"
#include "base/math/vector3.hpp"
#include "photon_grid.hpp"

namespace thread {
class Pool;
}

namespace scene::material {
class Sample;
}

namespace rendering::integrator::photon {

class Map {
  public:
    Map(uint32_t num_photons, float photon_radius, uint32_t num_workers);
    ~Map();

    void prepare();

    void insert(Photon const& photon, uint32_t index);

    void compile(uint32_t num_paths, math::AABB const& aabb, thread::Pool& pool);

    float3 li(f_float3 position, scene::material::Sample const& sample) const;

    size_t num_bytes() const;

  private:
    uint32_t num_paths_;

    uint32_t num_photons_;
    Photon*  photons_;

    float photon_radius_;

    math::AABB aabb_;

    uint32_t* num_reduced_;

    Grid caustic_grid_;
    Grid indirect_grid_;
};

}  // namespace rendering::integrator::photon

#endif
