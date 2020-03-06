#ifndef SU_CORE_RENDERING_POSTPROCESSOR_TONEMAPPING_GENERIC_HPP
#define SU_CORE_RENDERING_POSTPROCESSOR_TONEMAPPING_GENERIC_HPP

#include "tonemapper.hpp"

namespace rendering::postprocessor::tonemapping {

class Generic : public Tonemapper {
  public:
    Generic(float exposure, float contrast, float shoulder, float mid_in, float mid_out,
            float hdr_max);

  private:
    void apply(uint32_t id, uint32_t pass, int32_t begin, int32_t end, image::Float4 const& source,
               image::Float4& destination) final;

    float tonemap(float x) const;

    float a_;
    float b_;
    float c_;
    float d_;

    float hdr_max_;
};

}  // namespace rendering::postprocessor::tonemapping

#endif
