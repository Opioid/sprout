#include "aces.hpp"
#include "base/math/matrix3x3.inl"
#include "base/math/vector4.inl"
#include "base/spectrum/aces.hpp"
#include "image/typed_image.hpp"

namespace rendering::postprocessor::tonemapping {

// https://github.com/TheRealMJP/BakingLab/blob/master/BakingLab/ACES.hlsl

Aces::Aces(bool auto_expose, float exposure) : Tonemapper(auto_expose, exposure) {}

void Aces::apply(uint32_t /*id*/, uint32_t /*pass*/, int32_t begin, int32_t end,
                 image::Float4 const& source, image::Float4& destination, Scene const& /*scene*/) {
    float const factor = exposure_factor_;

    for (int32_t i = begin; i < end; ++i) {
        float4_p color = source.at(i);

        float3 const scaled = factor * color.xyz();

#ifdef SU_ACESCG
        float3 const rrt = spectrum::AP1_to_RRT_SAT(scaled);
#else
        float3 const rrt = spectrum::sRGB_to_RRT_SAT(scaled);
#endif
        float3 const odt  = spectrum::RRT_and_ODT(rrt);
        float3 const srgb = spectrum::ODT_SAT_to_sRGB(odt);

        destination.store(i, float4(srgb, color[3]));
    }
}

}  // namespace rendering::postprocessor::tonemapping
