#ifndef SU_EXTENSION_PROCEDURAL_FLUID_MATERIAL_HPP
#define SU_EXTENSION_PROCEDURAL_FLUID_MATERIAL_HPP

#include "fluid_simulation.hpp"
#include "scene/material/volumetric/volumetric_grid.hpp"

namespace image {
class Image;
}

namespace scene::prop {
class Prop;
}

namespace procedural::fluid {

class Material : public scene::material::volumetric::Grid_color {
  public:
    using Sampler_settings = scene::material::Sampler_settings;

    Material(Sampler_settings const& sampler_settings) noexcept;

    ~Material() noexcept override;

    void set_prop(scene::prop::Prop* prop) noexcept;

    void simulate(uint64_t start, uint64_t end, uint64_t frame_length,
                  thread::Pool& pool) noexcept override final;

    bool is_animated() const noexcept override final;

    Simulation& simulation() noexcept;

  private:
    image::Image*            image_;
    image::texture::Texture* texture_;

    scene::prop::Prop* prop_;

    Simulation sim_;

    uint32_t current_frame_;
};

}  // namespace procedural::fluid

#endif
