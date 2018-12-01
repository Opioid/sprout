#include "uncharted.hpp"
#include "base/math/vector4.inl"
#include "image/typed_image.inl"

namespace rendering {
namespace postprocessor {
namespace tonemapping {

Uncharted::Uncharted(float hdr_max)
    : normalization_factor_(normalization_factor(hdr_max, tonemap_function(hdr_max))) {}

void Uncharted::apply(uint32_t /*id*/, uint32_t /*pass*/, int32_t begin, int32_t end,
                      image::Float4 const& source, image::Float4& destination) {
    float norm = normalization_factor_;
    for (int32_t i = begin; i < end; ++i) {
        float4 const& color = source.at(i);

        destination.store(
            i, float4(norm * tonemap_function(color[0]), norm * tonemap_function(color[1]),
                      norm * tonemap_function(color[2]), color[3]));
    }
}

float Uncharted::tonemap_function(float x) {
    // Uncharted like in http://filmicgames.com/archives/75
    float a = 0.22f;
    float b = 0.30f;
    float c = 0.10f;
    float d = 0.20f;
    float e = 0.01f;
    float f = 0.30f;

    float ax = a * x;

    return ((x * (ax + c * b) + d * e) / (x * (ax + b) + d * f)) - e / f;
}

}  // namespace tonemapping
}  // namespace postprocessor
}  // namespace rendering
