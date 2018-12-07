#ifndef SU_CORE_SCENE_MATERIAL_VOLUMETRIC_GRID_HPP
#define SU_CORE_SCENE_MATERIAL_VOLUMETRIC_GRID_HPP

#include "image/texture/texture_adapter.hpp"
#include "volumetric_material.hpp"
#include "volumetric_octree.hpp"

namespace scene::material::volumetric {

class Grid final : public Material {
  public:
    Grid(Sampler_settings const& sampler_settings, Texture_adapter const& grid) noexcept;

    ~Grid() noexcept override final;

    float3 evaluate_radiance(float3 const& wi, float3 const& uvw, float volume, Filter filter,
                             Worker const& worker) const noexcept override final;

    CC collision_coefficients() const noexcept override final;

    CC collision_coefficients(float2 uv, Filter filter, Worker const& worker) const
        noexcept override final;

    CC collision_coefficients(float3 const& uvw, Filter filter, Worker const& worker) const
        noexcept override final;

    CCE collision_coefficients_emission(float3 const& uvw, Filter filter,
                                        Worker const& worker) const noexcept override final;

    void compile(thread::Pool& pool) noexcept override final;

    Gridtree const* volume_tree() const noexcept override final;

    bool is_heterogeneous_volume() const noexcept override final;

    size_t num_bytes() const noexcept override final;

  private:
    float density(float3 const& uvw, Filter filter, Worker const& worker) const noexcept;

    Texture_adapter grid_;

    Gridtree tree_;
};

}  // namespace scene::material::volumetric

#endif
