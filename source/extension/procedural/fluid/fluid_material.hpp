#ifndef SU_EXTENSION_PROCEDURAL_FLUID_MATERIAL_HPP
#define SU_EXTENSION_PROCEDURAL_FLUID_MATERIAL_HPP

#include "fluid_simulation.hpp"
#include "scene/material/volumetric/volumetric_grid.hpp"

namespace procedural::fluid {

class Material : public scene::material::volumetric::Grid {
  public:
    using Sampler_settings = scene::material::Sampler_settings;

    Material(Sampler_settings const& sampler_settings, Texture_adapter const& density) noexcept;

    ~Material() noexcept override;

    void simulate(uint64_t start, uint64_t end, uint64_t frame_length,
                  thread::Pool& pool) noexcept override final;

    bool is_animated() const noexcept override final;

    Simulation& simulation() noexcept;

  private:
    Simulation sim_;

    uint32_t current_frame_;
};

}  // namespace procedural::fluid

#endif
