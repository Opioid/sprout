#ifndef SU_RENDERING_INTEGRATOR_PARTICLE_PHOTON_SPARSE_GRID_HPP
#define SU_RENDERING_INTEGRATOR_PARTICLE_PHOTON_SPARSE_GRID_HPP

#include "base/math/aabb.hpp"
#include "base/math/vector2.hpp"
#include "base/math/vector3.hpp"

namespace thread {
class Pool;
}

using Threads = thread::Pool;

namespace scene {

namespace material {
class Sample;
}

namespace prop {
struct Intersection;
}

class Worker;

}  // namespace scene

namespace rendering::integrator::particle::photon {

struct Photon;
struct Photon_ref;

class Sparse_grid {
  public:
    using Intersection    = scene::prop::Intersection;
    using Material_sample = scene::material::Sample;

    Sparse_grid(float search_radius, float grid_cell_factor, bool check_disk);

    ~Sparse_grid();

    void resize(AABB const& aabb);

    void init_cells(uint32_t num_photons, Photon* photons);

    uint32_t reduce_and_move(Photon* photons, float merge_radius, uint32_t* num_reduced,
                             Threads& threads);

    void set_num_paths(uint64_t num_paths);

    float3 li(Intersection const& intersection, Material_sample const& sample,
              Photon_ref* photon_refs, scene::Worker const& worker) const;

  private:
    void release();

    void set(int3 const& c, int32_t value);

    uint32_t reduce(float merge_radius, int32_t begin, int32_t end);

    int32_t map1(float3 const& v) const;

    int3 map3(float3 const& v) const;

    int3 map3(float3 const& v, float2 cell_bound, uint8_t& adjacents) const;

    struct Adjacency {
        uint32_t num_cells;
        int2     cells[4];
    };

    void adjacent_cells(float3 const& v, float2 cell_bound, Adjacency& adjacency) const;

    static int32_t constexpr Log2_cell_dim = 5;
    static int32_t constexpr Cell_dim      = 1 << Log2_cell_dim;

    uint32_t num_photons_;
    Photon*  photons_;

    AABB aabb_;

    float search_radius_;

    float grid_cell_factor_;

    float surface_normalization_;
    float volume_normalization_;

    float2 cell_bound_;

    int3 dimensions_;

    int3 num_cells_;

    float3 local_to_texture_;

    struct Cell {
        int32_t* data;
        int32_t  value;
    };

    Cell* cells_;

    Adjacency adjacencies_[43];

    bool check_disk_;
};

}  // namespace rendering::integrator::particle::photon

#endif
