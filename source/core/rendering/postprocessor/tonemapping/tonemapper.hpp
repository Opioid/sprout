#ifndef SU_CORE_RENDERING_POSTPROCESSOR_TONEMAPPING_TONEMAPPER_HPP
#define SU_CORE_RENDERING_POSTPROCESSOR_TONEMAPPING_TONEMAPPER_HPP

#include "rendering/postprocessor/postprocessor.hpp"

namespace rendering::postprocessor::tonemapping {

class Tonemapper : public Postprocessor {
  public:
    Tonemapper(float exposure);

    ~Tonemapper() override;

    void init(Camera const& camera, thread::Pool& threads) final;

  protected:
    static float normalization_factor(float linear_max, float tonemapped_max);

    float exposure_factor_;
};

}  // namespace rendering::postprocessor::tonemapping

#endif
