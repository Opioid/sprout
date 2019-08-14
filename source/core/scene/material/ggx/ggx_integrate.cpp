#include "ggx_integrate.hpp"
#include "base/math/sample_distribution.inl"
#include "ggx.inl"

#include "image/encoding/png/png_writer.hpp"

#include <fstream>

// https://blog.selfshadow.com/2018/06/04/multi-faceted-part-2/
// https://google.github.io/filament/Filament.html#materialsystem/improvingthebrdfs/energylossinspecularreflectance

namespace scene::material::ggx {

float f_ss(float n_dot_wi, float n_dot_wo, float n_dot_h, float alpha) noexcept {
    float const alpha2 = std::max(alpha * alpha, Min_alpha2);

    float const  d = distribution_isotropic(n_dot_h, alpha2);
    float2 const g = optimized_masking_shadowing_and_g1_wo(n_dot_wi, n_dot_wo, alpha2);

    float const pdf = pdf_visible(d, g[1]);

    return (n_dot_wi * d * g[0]) / pdf;
}

float integrate_f_ss(float alpha, float n_dot_wo, uint32_t num_samples) {
    Layer layer;
    layer.set_tangent_frame(float3(1.f, 0.f, 0.f), float3(0.f, 1.f, 0.f), float3(0.f, 0.f, 1.f));

    n_dot_wo = clamp(n_dot_wo);

    // (sin, 0, cos)
    float3 const wo(std::sqrt(1.f - n_dot_wo * n_dot_wo), 0.f, n_dot_wo);

    float f = 0.f;

    for (uint32_t i = 0; i < num_samples; ++i) {
        float2 const xi = hammersley(i, num_samples, 0);

        float        n_dot_h;
        float3 const h = Isotropic::sample(wo, layer, alpha, xi, n_dot_h);

        float3 const wi = normalize(2.f * dot(wo, h) * h - wo);

        if (wi[2] < 0.f) {
            continue;
        }

        float const n_dot_wi = clamp(wi[2]);

        f += f_ss(n_dot_wi, n_dot_wo, n_dot_h, alpha);
    }

    return f / static_cast<float>(num_samples);
}

void integrate() noexcept {
    uint32_t constexpr Num_samples = 32;

    std::ofstream stream("../source/core/scene/material/ggx/ggx_integral.inl");

    stream.precision(6);
    stream << std::fixed;

    stream << "uint32_t constexpr E_max = " << Num_samples - 1 << ";\n\n";

    stream << "float constexpr E_scale = " << Num_samples << ".f;\n\n";

    stream << "float constexpr E[" << Num_samples << "][" << Num_samples << "] = {\n";

    float constexpr step = 1.f / static_cast<float>(Num_samples);

    float alpha = /*0.f;//*/ 0.5f * step;

    for (uint32_t a = 0; a < Num_samples; ++a) {
        stream << "\t// alpha " << alpha << std::endl;
        stream << "\t{\n\t\t";

        float n_dot_wo = 0.5f * step;

        for (uint32_t i = 0; i < Num_samples; ++i) {
            float const e = integrate_f_ss(alpha, n_dot_wo, 1024);

            stream << e << "f";

            if (i < Num_samples - 1) {
                stream << ", ";

                if (i > 0 && 0 == ((i + 1) % 8)) {
                    stream << "\n\t\t";
                }

                n_dot_wo += step;
            } else {
                stream << "\n\t}";
            }
        }

        if (a < Num_samples - 1) {
            stream << ",\n\n";
        } else {
            stream << "\n";
        }

        alpha += step;
    }

    stream << "};" << std::endl;

    //    image::encoding::png::Writer::write("e.png", &E[0][0], int2(Num_samples), 1.f, false);
}

}  // namespace scene::material::ggx
