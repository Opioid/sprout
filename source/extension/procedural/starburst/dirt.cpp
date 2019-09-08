#include "dirt.hpp"
#include "core/image/procedural/image_renderer.inl"

namespace procedural {
namespace starburst {

Dirt::Dirt(int2 dimensions, int32_t sqrt_num_samples)
    : image::procedural::Typed_renderer<float>(dimensions, sqrt_num_samples) {}

void Dirt::draw_concentric_circles(float2 pos, uint32_t num_circles, float thickness,
                                   float inner_color, float outer_color) {
    bool even = num_circles % 2 == 0;

    float radius = float(num_circles) * thickness;

    for (uint32_t i = 0; i < num_circles; ++i, even = !even, radius -= thickness) {
        set_brush(even ? outer_color : inner_color);
        draw_circle(pos, radius);
    }
}

}  // namespace starburst
}  // namespace procedural
