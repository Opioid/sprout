#include "linear.hpp"
#include "base/math/vector4.inl"
#include "image/typed_image.hpp"

namespace rendering::postprocessor::tonemapping {

Linear::Linear(bool auto_expose, float exposure) : Tonemapper(auto_expose, exposure) {}

void Linear::apply(uint32_t /*id*/, uint32_t /*pass*/, int32_t begin, int32_t end,
                   image::Float4 const& source, image::Float4& destination) {
    float const factor = exposure_factor_;

    for (int32_t i = begin; i < end; ++i) {
        float4 const& color = source.at(i);

        destination.store(i, float4(factor * color.xyz(), color[3]));
    }
}

}  // namespace rendering::postprocessor::tonemapping
