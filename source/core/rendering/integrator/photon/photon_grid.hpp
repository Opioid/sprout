#ifndef SU_RENDERING_INTEGRATOR_PHOTON_GRID_HPP
#define SU_RENDERING_INTEGRATOR_PHOTON_GRID_HPP

#include "base/math/aabb.hpp"
#include "base/math/vector3.hpp"

namespace thread {
class Pool;
}

namespace scene::material {
class Sample;
}

namespace rendering::integrator::photon {

struct Photon {
    float3 p;
    //    float3 n;
    float3 wi;
    float3 alpha;
    bool   caustic;
};

class Grid {
  public:
    Grid();
    ~Grid();

    void resize(math::AABB const& aabb, float radius);

    void update(uint32_t num_photons, Photon* photons, bool needs_sorting);

    uint32_t reduce(uint32_t* num_reduced, thread::Pool& pool);

    float3 li(f_float3 position, scene::material::Sample const& sample, uint32_t num_paths) const;

    size_t num_bytes() const;

  private:
    uint32_t reduce(int32_t begin, int32_t end);

    int32_t map1(f_float3 v) const;

    int3 map3(f_float3 v) const;
    int3 map3(f_float3 v, int8_t adjacent[3]) const;

    void adjacent_cells(f_float3 v, int2 cells[4]) const;

    uint32_t num_photons_;
    Photon*  photons_;

    float3 min_;

    float photon_radius_;
    float inverse_cell_size_;

    int3 dimensions_;
    int3 max_coords_;

    int2* grid_;

    static int32_t constexpr o_m1__0__0_ = -1;
    static int32_t constexpr o_p1__0__0_ = +1;

    int32_t o__0_m1__0_;
    int32_t o__0_p1__0_;

    int32_t o__0__0_m1_;
    int32_t o__0__0_p1_;

    int32_t o_m1_m1__0_;
    int32_t o_m1_p1__0_;

    int32_t o_p1_m1__0_;
    int32_t o_p1_p1__0_;

    int32_t o_m1_m1_m1_;
    int32_t o_m1_m1_p1_;
    int32_t o_m1_p1_m1_;
    int32_t o_m1_p1_p1_;

    int32_t o_p1_m1_m1_;
    int32_t o_p1_m1_p1_;
    int32_t o_p1_p1_m1_;
    int32_t o_p1_p1_p1_;

    int32_t o_m1__0_m1_;
    int32_t o_m1__0_p1_;
    int32_t o_p1__0_m1_;
    int32_t o_p1__0_p1_;

    int32_t o__0_m1_m1_;
    int32_t o__0_m1_p1_;
    int32_t o__0_p1_m1_;
    int32_t o__0_p1_p1_;
};

}  // namespace rendering::integrator::photon

#endif
