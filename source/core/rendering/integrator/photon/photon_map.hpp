#ifndef SU_RENDERING_INTEGRATOR_PHOTON_MAP_HPP
#define SU_RENDERING_INTEGRATOR_PHOTON_MAP_HPP

#include "base/math/aabb.hpp"
#include "base/math/vector3.hpp"
#include "photon_grid.hpp"

namespace scene {
class Scene;
}

namespace rendering::integrator::photon {

class Importance;

class Map {
  public:
    using Intersection    = scene::prop::Intersection;
    using Material_sample = scene::material::Sample;

    Map(uint32_t num_photons, float search_radius, float merge_radius, float coarse_search_radius,
        bool separate_indirect) noexcept;

    ~Map() noexcept;

    void init(scene::Scene const& scene, uint32_t num_workers) noexcept;

    void start() noexcept;

    void insert(Photon const& photon, uint32_t index) noexcept;

    void increment_importance(uint32_t light_id, float2 uv) noexcept;

    uint32_t compile_iteration(uint64_t num_paths, thread::Pool& pool) noexcept;

    void compile_finalize() noexcept;

    void export_importances() const noexcept;

    float3 li(Intersection const& intersection, Material_sample const& sample,
              scene::Worker const& worker) const noexcept;

    bool caustics_only() const noexcept;

    size_t num_bytes() const noexcept;

  private:
    AABB calculate_aabb(thread::Pool& pool) const noexcept;

    uint64_t num_caustic_paths_;
    uint64_t num_indirect_paths_;

    uint32_t num_photons_;

    uint32_t red_num_fine_;
    uint32_t red_num_coarse_;

    Photon* photons_;

    bool separate_indirect_;

    bool caustic_only_;

    float merge_radius_;

    AABB* aabbs_;

    uint32_t* num_reduced_;

    Grid fine_grid_;
    Grid coarse_grid_;

    Photon_ref* photon_refs_;

    uint32_t    num_importances_;
    Importance* importances_;
};

}  // namespace rendering::integrator::photon

#endif
