#ifndef SU_CORE_SCENE_MATERIAL_VOLUMETRIC_GRID_HPP
#define SU_CORE_SCENE_MATERIAL_VOLUMETRIC_GRID_HPP

#include "base/math/distribution/distribution_3d.hpp"
#include "base/math/interpolated_function_1d.hpp"
#include "image/texture/texture_adapter.hpp"
#include "volumetric_material.hpp"
#include "volumetric_octree.hpp"

namespace scene::material::volumetric {

class Grid : public Material {
  public:
    Grid(Sampler_settings const& sampler_settings, Texture_adapter const& density);

    ~Grid() override;

    float3 evaluate_radiance(float3 const& wi, float3 const& uvw, float volume, Filter filter,
                             Worker const& worker) const override;

    CC collision_coefficients(float3 const& uvw, Filter filter, Worker const& worker) const final;

    CCE collision_coefficients_emission(float3 const& uvw, Filter filter,
                                        Worker const& worker) const override;

    void commit(thread::Pool& threads, Scene const& scene) override;

    Gridtree const* volume_tree() const final;

    Boxi volume_texture_space_bounds(Scene const& scene) const final;

  protected:
    float density(float3 const& uvw, Filter filter, Worker const& worker) const;

    Texture_adapter density_;

    Gridtree tree_;
};

class Grid_emission : public Grid {
  public:
    Grid_emission(Sampler_settings const& sampler_settings, Texture_adapter const& grid);

    ~Grid_emission() override;

    float3 average_radiance(float volume, Scene const& scene) const final;

    float3 evaluate_radiance(float3 const& wi, float3 const& uvw, float volume, Filter filter,
                             Worker const& worker) const final;

    CCE collision_coefficients_emission(float3 const& uvw, Filter filter,
                                        Worker const& worker) const final;

    void commit(thread::Pool& threads, Scene const& scene) final;

    Sample_3D radiance_sample(float3 const& r3) const final;

    float emission_pdf(float3 const& uvw, Filter filter, Worker const& worker) const final;

    void prepare_sampling(Shape const& shape, uint32_t part, uint64_t time,
                          Transformation const& transformation, float area,
                          bool importance_sampling, thread::Pool& threads,
                          Scene const& scene) final;

    void set_temperature_map(Texture_adapter const& temperature_map);

  private:
    Texture_adapter temperature_;

    Interpolated_function_1D<float3> blackbody_;

    Distribution_3D distribution_;

    float3 average_emission_;

    float total_weight_;
};

class Grid_color : public Material {
  public:
    Grid_color(Sampler_settings const& sampler_settings);

    ~Grid_color() override;

    void set_color(Texture_adapter const& color);

    float3 evaluate_radiance(float3 const& wi, float3 const& uvw, float volume, Filter filter,
                             Worker const& worker) const final;

    CC collision_coefficients(float3 const& uvw, Filter filter, Worker const& worker) const final;

    CCE collision_coefficients_emission(float3 const& uvw, Filter filter,
                                        Worker const& worker) const final;

    void set_attenuation(float scattering_factor, float distance);

    void commit(thread::Pool& threads, Scene const& scene) final;

    Gridtree const* volume_tree() const final;

  protected:
    float4 color(float3 const& uvw, Filter filter, Worker const& worker) const;

    Texture_adapter color_;

    Gridtree tree_;

    float scattering_factor_;
};

}  // namespace scene::material::volumetric

#endif
