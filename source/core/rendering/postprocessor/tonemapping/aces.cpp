#include "aces.hpp"
#include "base/math/matrix3x3.inl"
#include "base/math/vector4.inl"
#include "image/typed_image.hpp"

#include <iostream>
#include "base/math/print.hpp"

namespace rendering::postprocessor::tonemapping {

Aces::Aces(float hdr_max)
    : normalization_factor_(normalization_factor(hdr_max, tonemap_function(hdr_max))) {}

void Aces::apply(uint32_t /*id*/, uint32_t /*pass*/, int32_t begin, int32_t end,
                 image::Float4 const& source, image::Float4& destination) {
    float const norm = normalization_factor_;
    for (int32_t i = begin; i < end; ++i) {
        float4 const& color = source.at(i);

        destination.store(
            i, float4(norm * tonemap_function(color[0]), norm * tonemap_function(color[1]),
                      norm * tonemap_function(color[2]), color[3]));
    }
}

float Aces::tonemap_function(float x) {
    // ACES like in https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/
    float constexpr a = 2.51f;
    float constexpr b = 0.03f;
    float constexpr c = 2.43f;
    float constexpr d = 0.59f;
    float constexpr e = 0.14f;

    return (x * (a * x + b)) / (x * (c * x + d) + e);
}

Aces_MJP::Aces_MJP() {}

// The code in this file was originally written by Stephen Hill (@self_shadow), who deserves all
// credit for coming up with this fit and implementing it. Buy him a beer next time you see him. :)

// sRGB => XYZ => D65_2_D60 => AP1 => RRT_SAT
float3x3 constexpr ACESInputMat(0.59719f, 0.35458f, 0.04823f, 0.07600f, 0.90834f, 0.01566f,
                                0.02840f, 0.13383f, 0.83777f);

// ODT_SAT => XYZ => D60_2_D65 => sRGB
float3x3 constexpr ACESOutputMat(

    1.60475f, -0.53108f, -0.07367f, -0.10208f, 1.10813f, -0.00605f, -0.00327f, -0.07276f, 1.07602f);

float3 RRTAndODTFit(float3 v) {
    float3 a = v * (v + 0.0245786f) - 0.000090537f;
    float3 b = v * (0.983729f * v + 0.4329510f) + 0.238081f;
    return a / b;
}

float3 ACESFitted(float3 color) {
    color = transform_vector(ACESInputMat, color);

    // Apply RRT and ODT
    color = RRTAndODTFit(color);

    color = transform_vector(ACESOutputMat, color);

    // Clamp to [0, 1]
    color = saturate(color);

    return color;
}

void Aces_MJP::apply(uint32_t /*id*/, uint32_t /*pass*/, int32_t begin, int32_t end,
                     image::Float4 const& source, image::Float4& destination) {
    float3       tmax = ACESFitted(float3(12.f));
    float3 const norm = float3(
        1.f);  // float3(normalization_factor(12.f, tmax[0]), normalization_factor(12.f, tmax[1]),
               // normalization_factor(12.f, tmax[2]));

    for (int32_t i = begin; i < end; ++i) {
        float4 const& color = source.at(i);

        destination.store(i, float4(norm * ACESFitted(color.xyz()), color[3]));
    }
}

}  // namespace rendering::postprocessor::tonemapping
