#ifndef SU_RENDERING_INTEGRATOR_PHOTON_GRID_HPP
#define SU_RENDERING_INTEGRATOR_PHOTON_GRID_HPP

#include "base/flags/flags.hpp"
#include "base/math/aabb.hpp"
#include "base/math/vector3.hpp"

namespace thread {
class Pool;
}

namespace scene {

namespace material {
class Sample;
}

namespace prop {
struct Intersection;
}

class Worker;

}  // namespace scene

namespace rendering::integrator::photon {

struct Photon {
    float3 p;
    float3 wi;
    float  alpha[3];

    enum class Property { First_hit = 1 << 0, Volumetric = 1 << 1 };

    flags::Flags<Property> properties;
};

class Grid {
  public:
    using Intersection    = scene::prop::Intersection;
    using Material_sample = scene::material::Sample;

    Grid(float radius, float merge_radius_factor);
    ~Grid();

    void resize(math::AABB const& aabb);

    void update(uint32_t num_photons, Photon* photons);

    uint32_t reduce_and_move(Photon* photons, uint32_t* num_reduced, thread::Pool& pool);

    float3 li(Intersection const& intersection, const Material_sample& sample, uint32_t num_paths,
              scene::Worker const& worker) const;

    size_t num_bytes() const;

  private:
    uint32_t reduce(int32_t begin, int32_t end);

    int32_t map1(float3 const& v) const;

    int3 map3(float3 const& v) const;
    int3 map3(float3 const& v, int8_t adjacent[3]) const;

    void adjacent_cells(float3 const& v, int2 cells[4]) const;

    static float3 scattering_coefficient(Intersection const&  intersection,
                                         scene::Worker const& worker);

    uint32_t num_photons_;
    Photon*  photons_;

    math::AABB aabb_;

    float photon_radius_;
    float inverse_cell_size_;
    float merge_radius_factor_;

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
