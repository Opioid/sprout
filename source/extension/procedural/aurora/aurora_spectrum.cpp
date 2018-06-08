#include "aurora_spectrum.hpp"
#include "base/math/function/interpolated_function.inl"
#include "base/spectrum/interpolated.hpp"
#include "base/spectrum/xyz.hpp"

#include <iostream>
#include "base/math/print.hpp"

namespace procedural::aurora::spectrum {

using namespace ::spectrum;

math::Interpolated_function<float3> color;

void init() {
    const Interpolated CIE_X(CIE_Wavelengths_360_830_1nm, CIE_X_360_830_1nm, CIE_XYZ_Num);
    const Interpolated CIE_Y(CIE_Wavelengths_360_830_1nm, CIE_Y_360_830_1nm, CIE_XYZ_Num);
    const Interpolated CIE_Z(CIE_Wavelengths_360_830_1nm, CIE_Z_360_830_1nm, CIE_XYZ_Num);

    constexpr float wl_427 = 427.8f;
    float3 const    cie_427(CIE_X.evaluate(wl_427), CIE_Y.evaluate(wl_427), CIE_Z.evaluate(wl_427));

    constexpr float wl_557 = 557.7f;
    float3 const    cie_557(CIE_X.evaluate(wl_557), CIE_Y.evaluate(wl_557), CIE_Z.evaluate(wl_557));

    constexpr float wl_630 = 630.f;
    float3 const    cie_630(CIE_X.evaluate(wl_630), CIE_Y.evaluate(wl_630), CIE_Z.evaluate(wl_630));

    constexpr float intensities_427[] = {0.f, 0.21f, 0.39f, 0.49f, 0.505f, 0.47f,
                                         // 160
                                         0.4f, 0.33f, 0.28f, 0.21f, 0.19f, 0.12f,
                                         // 220
                                         0.09f, 0.05f, 0.01f, 0.f, 0.f, 0.f,
                                         // 280
                                         0.f, 0.f,

                                         0.f};

    /*	constexpr float intensities_427[] = {
                    0.f,
                    0.1f,
                    0.15f,
                    0.2f,
                    0.25f,
                    0.225f,
                    // 160
                    0.2f,
                    0.18f,
                    0.18f,
                    0.18f,
                    0.18f,
                    0.18f,
                    // 220
                    0.18f,
                    0.18f,
                    0.18f,
                    0.15f,
                    0.14f,
                    0.12f,
                    // 280
                    0.1f,
                    0.05f,

                    0.01f
            };*/

    constexpr float intensities_557[] = {0.f, 0.42f, 0.79f, 0.95f, 1.f, 0.91f,
                                         // 160,
                                         0.815f, 0.7f, 0.6f, 0.48f, 0.38f, 0.29f,
                                         // 220
                                         0.205f, 0.12f, 0.06f, 0.025f, 0.f, 0.f,
                                         // 280
                                         0.f, 0.f,

                                         0.f};

    constexpr float intensities_630[] = {0.f, 0.f, 0.f, 0.f, 0.f, 0.f,
                                         // 160
                                         0.f, 0.0125f, 0.05f, 0.08f, 0.099f, 0.1f,
                                         // 220
                                         0.11f, 0.12f, 0.16f, 0.2f, 0.205f, 0.21f,
                                         // 280
                                         0.202f, 0.19f,

                                         0.15f};

    constexpr size_t num_elements = sizeof(intensities_427) / sizeof(float);

    constexpr float step = (630.f - 427.8f) / static_cast<float>(3);

    constexpr float normalization = 1.f / 106.856895f;

    float3 colors[num_elements];

    for (size_t i = 0; i < num_elements; ++i) {
        float3 xyz(0.f);

        xyz += 0.f * intensities_427[i] * cie_427;
        xyz += std::min(1.f * intensities_557[i], 1.f) * cie_557;
        xyz += intensities_630[i] * cie_630;

        xyz *= normalization * step;

        float3 const rgb = XYZ_to_linear_RGB(xyz);

        colors[i] = math::max(rgb, 0.f);
    }

    color.from_array(0.f, 1.f, num_elements, colors);
}

float3 linear_rgb(float normalized_height) {
    return color(normalized_height);
}

}  // namespace procedural::aurora::spectrum
