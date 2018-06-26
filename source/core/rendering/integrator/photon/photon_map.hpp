#ifndef SU_RENDERING_INTEGRATOR_PHOTON_MAP_HPP
#define SU_RENDERING_INTEGRATOR_PHOTON_MAP_HPP

#include "base/math/aabb.hpp"
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
    Map(uint32_t num_photons, float photon_radius);
    ~Map();

    void insert(Photon const& photon, uint32_t index);

    void compile(uint32_t num_paths, math::AABB const& aabb);

    float3 li(f_float3 position, scene::material::Sample const& sample) const;

  private:
    int32_t map(f_float3 v) const;
    int3    map(f_float3 v, int8_t adjacent[3]) const;
    int32_t map(f_int3 c) const;

    void adjacent_cells(f_float3 v, int2 cells[4]) const;

    uint32_t num_paths_;

    uint32_t num_photons_;
    Photon*  photons_;

    float photon_radius_;

    float inverse_cell_size_;

    math::AABB aabb_;

    int3 grid_dimensions_;

    int32_t grid_area_;

    int2* grid_;

    static int32_t constexpr o_m1__0__0_ = -1;
    static int32_t constexpr o_p1__0__0_ = +1;

    int32_t o__0_m1__0_;
    int32_t o__0_p1__0_;
    int32_t o__0__0_m1_;
    int32_t o__0__0_p1_;
};

}  // namespace rendering::integrator::photon

#endif
