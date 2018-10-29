#pragma once

#include "core/image/procedural/image_renderer.hpp"

namespace procedural {
namespace starburst {

class Dirt : public image::procedural::Typed_renderer<float> {
  public:
    Dirt(int2 dimensions, int32_t sqrt_num_samples = 1);

    void draw_concentric_circles(float2 pos, uint32_t num_circles, float thickness,
                                 float inner_color, float outer_color);

  private:
};

}  // namespace starburst
}  // namespace procedural
