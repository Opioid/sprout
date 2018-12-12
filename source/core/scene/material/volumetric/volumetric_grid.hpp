#ifndef SU_CORE_SCENE_MATERIAL_VOLUMETRIC_GRID_HPP
#define SU_CORE_SCENE_MATERIAL_VOLUMETRIC_GRID_HPP

#include "base/math/distribution/distribution_3d.hpp"
#include "image/texture/texture_adapter.hpp"
#include "volumetric_material.hpp"
#include "volumetric_octree.hpp"

namespace scene::material::volumetric {

class Grid : public Material {
  public:
    Grid(Sampler_settings const& sampler_settings, Texture_adapter const& density) noexcept;

    ~Grid() noexcept override;

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

  protected:
    float density(float3 const& uvw, Filter filter, Worker const& worker) const noexcept;

    Texture_adapter density_;

    Gridtree tree_;
};

class Grid_emission : public Grid {
  public:
    Grid_emission(Sampler_settings const& sampler_settings, Texture_adapter const& grid) noexcept;

    ~Grid_emission() noexcept override;

    bool has_emission_map() const noexcept override final;

    Sample_3D radiance_sample(float3 const& r2) const noexcept override final;

    float emission_pdf(float3 const& uvw, Filter filter, Worker const& worker) const
        noexcept override final;

    void prepare_sampling(Shape const& shape, uint32_t part, uint64_t time,
                          Transformation const& transformation, float area,
                          bool importance_sampling, thread::Pool& pool) noexcept override final;

  private:
    math::Distribution_3D distribution_;

    float3 average_emission_;

    float total_weight_;
};

}  // namespace scene::material::volumetric

#endif
