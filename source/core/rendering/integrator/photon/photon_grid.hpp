#ifndef SU_RENDERING_INTEGRATOR_PHOTON_GRID_HPP
#define SU_RENDERING_INTEGRATOR_PHOTON_GRID_HPP

#include "base/flags/flags.hpp"
#include "base/math/aabb.hpp"
#include "base/math/vector2.hpp"
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

    Grid(float search_radius, float grid_radius_factor) noexcept;

    ~Grid() noexcept;

    void resize(AABB const& aabb) noexcept;

    void init_cells(uint32_t num_photons, Photon* photons) noexcept;

    uint32_t reduce_and_move(Photon* photons, float merge_radius, uint32_t* num_reduced,
                             thread::Pool& pool) noexcept;

    float3 li(Intersection const& intersection, const Material_sample& sample, uint32_t num_paths,
              scene::Worker const& worker) const noexcept;

    size_t num_bytes() const noexcept;

  private:
    uint32_t reduce(float merge_radius, int32_t begin, int32_t end) noexcept;

    uint8_t adjacent(float s) const noexcept;

    int32_t map1(float3 const& v) const noexcept;

    int3 map3(float3 const& v, uint8_t& adjacents) const noexcept;

    struct Adjacency {
        int2     cells[4];
        uint32_t num_cells;
    };

    void adjacent_cells(float3 const& v, Adjacency& adjacency) const noexcept;

    uint32_t num_photons_;
    Photon*  photons_;

    AABB aabb_;

    float search_radius_;

    float grid_cell_factor_;
    float lower_cell_bound_;
    float upper_cell_bound_;

    int3 dimensions_;

    float3 local_to_texture_;

    int32_t* grid_;

    Adjacency adjacencies_[43];
};

}  // namespace rendering::integrator::photon

#endif
