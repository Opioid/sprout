#ifndef SU_CORE_RENDERING_POSTPROCESSOR_TONEMAPPING_UNCHARTED_HPP
#define SU_CORE_RENDERING_POSTPROCESSOR_TONEMAPPING_UNCHARTED_HPP

#include "tonemapper.hpp"

namespace rendering::postprocessor::tonemapping {

class Uncharted : public Tonemapper {
  public:
    Uncharted(float exposure, float hdr_max);

  private:
    void apply(uint32_t id, uint32_t pass, int32_t begin, int32_t end, image::Float4 const& source,
               image::Float4& destination) final;

    static float tonemap(float x);

    float normalization_factor_;
};

}  // namespace rendering::postprocessor::tonemapping

#endif
