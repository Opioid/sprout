#ifndef SU_CORE_RENDERING_POSTPROCESSOR_TONEMAPPING_TONEMAPPER_HPP
#define SU_CORE_RENDERING_POSTPROCESSOR_TONEMAPPING_TONEMAPPER_HPP

#include "rendering/postprocessor/postprocessor.hpp"

namespace rendering::postprocessor::tonemapping {

class Tonemapper : public Postprocessor {
  public:
    ~Tonemapper() override;

    void init(scene::camera::Camera const& camera, thread::Pool& pool) override final;

    size_t num_bytes() const override final;

  protected:
    static float normalization_factor(float hdr_max, float tonemapped_max);
};

}  // namespace rendering::postprocessor::tonemapping

#endif
