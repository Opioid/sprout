#ifndef SU_CORE_SCENE_MATERIAL_VOLUMETRIC_GRID_HPP
#define SU_CORE_SCENE_MATERIAL_VOLUMETRIC_GRID_HPP

#include "base/math/distribution_3d.hpp"
#include "base/math/interpolated_function_1d.hpp"
#include "volumetric_material.hpp"
#include "volumetric_octree.hpp"

namespace scene::material::volumetric {

class Grid : public Material {
  public:
    Grid(Sampler_settings sampler_settings, Texture const& density);

    ~Grid() override;

    float3 evaluate_radiance(float3_p wi, float3_p n, float3_p uvw, float volume, Filter filter,
                             Worker const& worker) const override;

    CC collision_coefficients(float3_p uvw, Filter filter, Worker const& worker) const final;

    CCE collision_coefficients_emission(float3_p uvw, Filter filter,
                                        Worker const& worker) const override;

    void commit(Threads& threads, Scene const& scene) override;

    Gridtree const* volume_tree() const final;

    Boxi volume_texture_space_bounds(Scene const& scene) const final;

  protected:
    float density(float3_p uvw, Filter filter, Worker const& worker) const;

    Texture density_;

    Gridtree tree_;
};

class Grid_emission : public Grid {
  public:
    Grid_emission(Sampler_settings sampler_settings, Texture const& grid);

    ~Grid_emission() override;

    float3 evaluate_radiance(float3_p wi, float3_p n, float3_p uvw, float volume, Filter filter,
                             Worker const& worker) const final;

    Radiance_sample radiance_sample(float3_p r3) const final;

    float emission_pdf(float3_p uvw, Worker const& worker) const final;

    CCE collision_coefficients_emission(float3_p uvw, Filter filter,
                                        Worker const& worker) const final;

    void commit(Threads& threads, Scene const& scene) final;

    float3 prepare_sampling(Shape const& shape, uint32_t part, Transformation const& trafo,
                            float area, bool importance_sampling, Threads& threads,
                            Scene const& scene) final;

    void set_temperature_map(Texture const& temperature_map);

  private:
    Texture temperature_;

    Interpolated_function_1D<float3> blackbody_;

    Distribution_3D distribution_;

    float3 average_emission_;

    float3 a_norm_;

    float pdf_factor_;
};

class Grid_color : public Material {
  public:
    Grid_color(Sampler_settings sampler_settings);

    ~Grid_color() override;

    void set_color(Texture const& color);

    float3 evaluate_radiance(float3_p wi, float3_p n, float3_p uvw, float volume, Filter filter,
                             Worker const& worker) const final;

    CC collision_coefficients(float3_p uvw, Filter filter, Worker const& worker) const final;

    CCE collision_coefficients_emission(float3_p uvw, Filter filter,
                                        Worker const& worker) const final;

    void set_volumetric(float scattering_factor, float distance, float anisotropy);

    void commit(Threads& threads, Scene const& scene) final;

    Gridtree const* volume_tree() const final;

  protected:
    float4 color(float3_p uvw, Filter filter, Worker const& worker) const;

    Texture color_;

    Gridtree tree_;

    float scattering_factor_;
};

}  // namespace scene::material::volumetric

#endif
