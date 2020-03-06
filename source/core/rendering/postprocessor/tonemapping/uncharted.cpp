#include "uncharted.hpp"
#include "base/math/vector4.inl"
#include "image/typed_image.hpp"

namespace rendering::postprocessor::tonemapping {

Uncharted::Uncharted(bool auto_expose, float exposure, float hdr_max)
    : Tonemapper(auto_expose, exposure),
      normalization_factor_(normalization_factor(hdr_max, tonemap(hdr_max))) {}

void Uncharted::apply(uint32_t /*id*/, uint32_t /*pass*/, int32_t begin, int32_t end,
                      image::Float4 const& source, image::Float4& destination) {
    float const factor = exposure_factor_;
    float const norm   = normalization_factor_;

    for (int32_t i = begin; i < end; ++i) {
        float4 const& color = source.at(i);

        destination.store(
            i, float4(norm * tonemap(factor * color[0]), norm * tonemap(factor * color[1]),
                      norm * tonemap(factor * color[2]), color[3]));
    }
}

float Uncharted::tonemap(float x) {
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

}  // namespace rendering::postprocessor::tonemapping
