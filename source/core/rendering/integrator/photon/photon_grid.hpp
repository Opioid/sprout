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

    Grid(float radius, float merge_radius_factor) noexcept;

    ~Grid() noexcept;

    void resize(math::AABB const& aabb) noexcept;

    void update(uint32_t num_photons, Photon* photons) noexcept;

    uint32_t reduce_and_move(Photon* photons, uint32_t* num_reduced, thread::Pool& pool) noexcept;

    float3 li(Intersection const& intersection, const Material_sample& sample, uint32_t num_paths,
              scene::Worker const& worker) const noexcept;

    size_t num_bytes() const noexcept;

  private:
    uint32_t reduce(int32_t begin, int32_t end) noexcept;

    static uint8_t adjacent(float s) noexcept;

    int32_t map1(float3 const& v) const noexcept;

    int3 map3(float3 const& v) const noexcept;
    int3 map3(float3 const& v, uint8_t& adjacent) const noexcept;

    struct Adjacency {
        int2 cells[4];
        uint32_t num_cells;
    };

    void adjacent_cells(float3 const& v, Adjacency& adjacency) const noexcept;

    static float3 scattering_coefficient(Intersection const&  intersection,
                                         scene::Worker const& worker) noexcept;

    uint32_t num_photons_;
    Photon*  photons_;

    math::AABB aabb_;

    static float constexpr Grid_radius_factor = 4.f;
    static float constexpr Lower_cell_bound = 1.f / Grid_radius_factor;
    static float constexpr Upper_cell_bound = 1.f - Lower_cell_bound;

    float photon_radius_;
    float inverse_cell_size_;
    float merge_radius_factor_;

    int3 dimensions_;

    int2* grid_;

    Adjacency adjacencies_[43];
};

}  // namespace rendering::integrator::photon

#endif
