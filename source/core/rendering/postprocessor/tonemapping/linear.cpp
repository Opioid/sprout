#include "linear.hpp"
#include "base/math/vector4.inl"
#include "base/spectrum/aces.hpp"
#include "image/typed_image.hpp"

namespace rendering::postprocessor::tonemapping {

Linear::Linear(bool auto_expose, float exposure) : Tonemapper(auto_expose, exposure) {}

void Linear::apply(uint32_t /*id*/, uint32_t /*pass*/, int32_t begin, int32_t end,
                   image::Float4 const& source, image::Float4& destination,
                   Scene const& /*scene*/) {
    float const factor = exposure_factor_;

    for (int32_t i = begin; i < end; ++i) {
        float4_p color = source.at(i);

#ifdef SU_ACESCG
        float3 const scaled = factor * color.xyz();

        float3 const srgb = spectrum::AP1_to_sRGB(scaled);
#else
        float3 const srgb = factor * color.xyz();
#endif

        destination.store(i, float4(srgb, color[3]));
    }
}

}  // namespace rendering::postprocessor::tonemapping
