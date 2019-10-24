#ifndef SU_RENDERING_INTEGRATOR_PARTICLE_PHOTON_MAP_HPP
#define SU_RENDERING_INTEGRATOR_PARTICLE_PHOTON_MAP_HPP

#include "base/math/aabb.hpp"
#include "base/math/vector3.hpp"
#include "photon_grid.hpp"
#include "photon_sparse_grid.hpp"

namespace scene {
class Scene;
}

namespace rendering::integrator::particle::photon {

class Map {
  public:
    using Intersection    = scene::prop::Intersection;
    using Material_sample = scene::material::Sample;

    Map(uint32_t num_photons, float search_radius, float merge_radius) noexcept;

    ~Map() noexcept;

    void init(uint32_t num_workers) noexcept;

    void start() noexcept;

    void insert(Photon const& photon, uint32_t index) noexcept;

    uint32_t compile_iteration(uint32_t num_photons, uint64_t num_paths,
                               thread::Pool& pool) noexcept;

    void compile_finalize() noexcept;

    float3 li(Intersection const& intersection, Material_sample const& sample,
              scene::Worker const& worker) const noexcept;

    bool caustics_only() const noexcept;

    size_t num_bytes() const noexcept;

  private:
    AABB calculate_aabb(uint32_t num_photons, thread::Pool& pool) const noexcept;

    uint64_t num_paths_;

    uint32_t num_photons_;

    uint32_t reduced_num_;

    Photon* photons_;

    bool caustic_only_;

    float merge_radius_;

    AABB* aabbs_;

    uint32_t* num_reduced_;

    Grid grid_;
};

}  // namespace rendering::integrator::particle::photon

#endif
