#ifndef SU_RENDERING_INTEGRATOR_PHOTON_MAP_HPP
#define SU_RENDERING_INTEGRATOR_PHOTON_MAP_HPP

#include "base/math/aabb.hpp"
#include "base/math/vector3.hpp"
#include "photon_grid.hpp"

namespace rendering::integrator::photon {

class Map {
  public:
    using Intersection    = scene::prop::Intersection;
    using Material_sample = scene::material::Sample;

    Map(uint32_t num_photons, float search_radius, float merge_radius, float indirect_radius_factor,
        bool separate_indirect) noexcept;

    ~Map() noexcept;

    void init(uint32_t num_workers) noexcept;

    void insert(Photon const& photon, uint32_t index) noexcept;

    uint32_t compile_iteration(uint32_t num_paths, thread::Pool& pool) noexcept;

    void compile_finalize() noexcept;

    float3 li(Intersection const& intersection, Material_sample const& sample,
              scene::Worker const& worker) const noexcept;

    size_t num_bytes() const noexcept;

  private:
    AABB calculate_aabb(thread::Pool& pool) const noexcept;

    uint32_t num_paths_;

    uint32_t num_photons_;
    Photon*  photons_;

    bool separate_indirect_;

    uint32_t* num_reduced_;

    Grid caustic_grid_;
    Grid indirect_grid_;
};

}  // namespace rendering::integrator::photon

#endif
