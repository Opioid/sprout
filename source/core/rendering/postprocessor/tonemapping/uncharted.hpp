#pragma once

#include "tonemapper.hpp"

namespace rendering {
namespace postprocessor {
namespace tonemapping {

class Uncharted : public Tonemapper {
  public:
    Uncharted(float hdr_max);

  private:
    virtual void apply(uint32_t id, uint32_t pass, int32_t begin, int32_t end,
                       image::Float4 const& source, image::Float4& destination) final;

    static float tonemap_function(float x);

    float normalization_factor_;
};

}  // namespace tonemapping
}  // namespace postprocessor
}  // namespace rendering
