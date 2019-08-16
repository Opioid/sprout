#include "ggx_integrate.hpp"
#include "base/math/sample_distribution.inl"
#include "ggx.inl"
#include "image/encoding/png/png_writer.hpp"
#include "scene/material/material_sample.inl"

#include <fstream>

#include <iostream>

// https://blog.selfshadow.com/2018/06/04/multi-faceted-part-2/
// https://google.github.io/filament/Filament.html#materialsystem/improvingthebrdfs/energylossinspecularreflectance

namespace scene::material::ggx {

float f_t_ss(float n_dot_wi, float n_dot_wo, float wi_dot_h, float wo_dot_h, float n_dot_h,
             float alpha, IoR ior, float f0) noexcept {
    fresnel::Schlick1 const schlick(f0);

    auto const ggx = ggx::Isotropic::refraction(n_dot_wi, n_dot_wo, wi_dot_h, wo_dot_h, n_dot_h,
                                                alpha, ior, schlick);

    return (std::min(n_dot_wi, n_dot_wo) * ggx.reflection[0]) / ggx.pdf;
}

float integrate_f_ss(float alpha, float n_dot_wo, uint32_t num_samples) noexcept {
    if (alpha < Min_alpha) {
        return 1.f;
    }

    fresnel::Schlick const schlick(1.f);

    Layer layer{float3(1.f, 0.f, 0.f), float3(0.f, 1.f, 0.f), float3(0.f, 0.f, 1.f)};

    n_dot_wo = clamp(n_dot_wo);

    // (sin, 0, cos)
    float3 const wo(std::sqrt(1.f - n_dot_wo * n_dot_wo), 0.f, n_dot_wo);

    float accum = 0.f;

    for (uint32_t i = 0; i < num_samples; ++i) {
        float2 const xi = hammersley(i, num_samples, 0);

        bxdf::Sample result;

        float const n_dot_wi = ggx::Isotropic::reflect(wo, n_dot_wo, layer, alpha, schlick, xi,
                                                       result);

        accum += (n_dot_wi * result.reflection[0]) / result.pdf;
    }

    return accum / static_cast<float>(num_samples);
}

float integrate_f_s_ss(float alpha, float ior_t, float n_dot_wo, uint32_t num_samples) noexcept {
    if (alpha < Min_alpha) {
        return 1.f;
    }

    Layer layer;
    layer.set_tangent_frame(float3(1.f, 0.f, 0.f), float3(0.f, 1.f, 0.f), float3(0.f, 0.f, 1.f));

    n_dot_wo = clamp(n_dot_wo);

    // (sin, 0, cos)
    float3 const wo(std::sqrt(1.f - n_dot_wo * n_dot_wo), 0.f, n_dot_wo);

    bool const same_side = dot(wo, layer.n_) > 0.f;

    IoR ior{ior_t, 1.f};

    ior = ior.swapped(same_side);

    float const f0 = fresnel::schlick_f0(ior.eta_i, ior.eta_t);

    float accum = 0.f;

    for (uint32_t i = 0; i < num_samples; ++i) {
        float2 const xi = hammersley(i, num_samples, 0);

        float        n_dot_h;
        float3 const h = ggx::Isotropic::sample(wo, layer, alpha, xi, n_dot_h);

        // float const n_dot_wo = layer.clamp_abs_n_dot(wo);

        float const wo_dot_h = clamp_dot(wo, h);

        float const eta = ior.eta_i / ior.eta_t;

        float const sint2 = (eta * eta) * (1.f - wo_dot_h * wo_dot_h);

        float f;
        float wi_dot_h;

        if (sint2 >= 1.f) {
            f        = 1.f;
            wi_dot_h = 0.f;
        } else {
            wi_dot_h = std::sqrt(1.f - sint2);

            float const cos_x = ior.eta_i > ior.eta_t ? wi_dot_h : wo_dot_h;

            f = fresnel::schlick(cos_x, f0);
        }

        bxdf::Sample result;

        {
            float const n_dot_wi = ggx::Isotropic::reflect(wo, h, n_dot_wo, n_dot_h, wi_dot_h,
                                                           wo_dot_h, layer, alpha, result);

            accum += (std::min(n_dot_wi, n_dot_wo) * f * result.reflection[0]) / result.pdf;
        }
        {
            float const r_wo_dot_h = same_side ? -wo_dot_h : wo_dot_h;

            float const n_dot_wi = ggx::Isotropic::refract(wo, h, n_dot_wo, n_dot_h, -wi_dot_h,
                                                           r_wo_dot_h, layer, alpha, ior, result);

            float const omf = 1.f - f;

            accum += (n_dot_wi * omf * result.reflection[0]) / result.pdf;
        }
    }

    return accum / static_cast<float>(num_samples);
}

void make_f_ss_table(std::ostream& stream) noexcept {
    uint32_t constexpr Num_samples = 32;

    stream << "uint32_t constexpr E_size = " << Num_samples << ";\n\n";

    stream << "float constexpr E[" << Num_samples << "][" << Num_samples << "] = {\n";

    float constexpr step = 1.f / static_cast<float>(Num_samples - 1);

    float alpha = 0.f;

    for (uint32_t a = 0; a < Num_samples; ++a) {
        stream << "\t// alpha " << alpha << std::endl;
        stream << "\t{\n\t\t";

        float n_dot_wo = 0.f;

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

void make_f_s_ss_table(std::ostream& stream) noexcept {
    uint32_t constexpr Num_samples = 32;

    stream << "uint32_t constexpr E_s_size = " << Num_samples << ";\n\n";

    stream << "float constexpr E_s[" << Num_samples << "][" << Num_samples << "] = {\n";

    float constexpr step = 1.f / static_cast<float>(Num_samples - 1);

    float alpha = 0.f;

    float ior = 1.5f;

    for (uint32_t a = 0; a < Num_samples; ++a) {
        stream << "\t// alpha " << alpha << std::endl;
        stream << "\t{\n\t\t";

        float n_dot_wo = 0.f;

        for (uint32_t i = 0; i < Num_samples; ++i) {
            float const e_s = integrate_f_s_ss(alpha, ior, n_dot_wo, 1024);

            stream << e_s << "f";

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
}

void integrate() noexcept {
    std::ofstream stream("../source/core/scene/material/ggx/ggx_integral.inl");

    stream.precision(6);
    stream << std::fixed;

    make_f_ss_table(stream);

    stream << std::endl;

    make_f_s_ss_table(stream);
}

}  // namespace scene::material::ggx
