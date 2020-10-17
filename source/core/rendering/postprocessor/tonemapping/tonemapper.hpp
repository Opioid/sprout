#ifndef SU_CORE_RENDERING_POSTPROCESSOR_TONEMAPPING_TONEMAPPER_HPP
#define SU_CORE_RENDERING_POSTPROCESSOR_TONEMAPPING_TONEMAPPER_HPP

#include "rendering/postprocessor/postprocessor.hpp"

namespace rendering::postprocessor::tonemapping {

class Tonemapper : public Postprocessor {
  public:
    Tonemapper(bool auto_expose, float exposure);

    ~Tonemapper() override;

    void init(Camera const& camera, Threads& threads) final;

  protected:
    void pre_apply(image::Float4 const& source, image::Float4& destination, Threads& threads) final;

    static float normalization_factor(float linear_max, float tonemapped_max);

    bool const auto_expose_;

    float exposure_factor_;
};

}  // namespace rendering::postprocessor::tonemapping

#endif
