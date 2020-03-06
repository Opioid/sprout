#ifndef SU_CORE_RENDERING_POSTPROCESSOR_TONEMAPPING_PIECEWISE_HPP
#define SU_CORE_RENDERING_POSTPROCESSOR_TONEMAPPING_PIECEWISE_HPP

#include "tonemapper.hpp"

// http://filmicworlds.com/blog/filmic-tonemapping-with-piecewise-power-curves/

namespace rendering::postprocessor::tonemapping {

class Piecewise : public Tonemapper {
  public:
    Piecewise(bool auto_expose, float exposure, float toe_strength, float toe_length,
              float shoulder_strength, float shoulder_length, float shoulder_angle);

  public:
    void apply(uint32_t id, uint32_t pass, int32_t begin, int32_t end, image::Float4 const& source,
               image::Float4& destination) final;

    struct CurveSegment {
        float eval(float x) const;

        float offset_x;
        float offset_y;
        float scale_x;  // always 1 or -1
        float scale_y;
        float ln_a;
        float b;
    };

    float eval(float x) const;

    float w_inv_;

    float x0_;
    float x1_;

    CurveSegment segments_[3];
};

}  // namespace rendering::postprocessor::tonemapping

#endif
