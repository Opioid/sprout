#include "ggx_integrate.hpp"
#include "base/math/sample_distribution.inl"
#include "ggx.inl"
#include "image/encoding/png/png_writer.hpp"
#include "scene/material/disney/disney.inl"
#include "scene/material/material_sample.inl"

#include <fstream>

#include <iostream>

// https://blog.selfshadow.com/2018/06/04/multi-faceted-part-2/
// https://google.github.io/filament/Filament.html#materialsystem/improvingthebrdfs/energylossinspecularreflectance

namespace scene::material::ggx {

float integrate_f_ss(float alpha, float n_dot_wo, uint32_t num_samples) {
    if (alpha < Min_alpha) {
        return 1.f;
    }

    fresnel::Schlick const schlick(1.f);

    Layer const layer{float3(1.f, 0.f, 0.f), float3(0.f, 1.f, 0.f), float3(0.f, 0.f, 1.f)};

    n_dot_wo = clamp(n_dot_wo);

    // (sin, 0, cos)
    float3 const wo(std::sqrt(1.f - n_dot_wo * n_dot_wo), 0.f, n_dot_wo);

    float accum = 0.f;

    for (uint32_t i = 0; i < num_samples; ++i) {
        float2 const xi = hammersley(i, num_samples, 0);

        bxdf::Sample result;

        float const n_dot_wi = ggx::Iso::reflect(wo, n_dot_wo, alpha, schlick, xi, layer, result);

        accum += (n_dot_wi * result.reflection[0]) / result.pdf;
    }

    return accum / float(num_samples);
}

float integrate_f_s_ss(float alpha, float ior_t, float n_dot_wo, uint32_t num_samples) {
    if (alpha < Min_alpha || ior_t <= 1.f) {
        return 1.f;
    }

    Layer const layer{float3(1.f, 0.f, 0.f), float3(0.f, 1.f, 0.f), float3(0.f, 0.f, 1.f)};

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
        float3 const h = ggx::Aniso::sample(wo, float2(alpha), xi, layer, n_dot_h);

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
            float const n_dot_wi = ggx::Iso::reflect(wo, h, n_dot_wo, n_dot_h, wi_dot_h, wo_dot_h,
                                                     alpha, layer, result);

            accum += (std::min(n_dot_wi, n_dot_wo) * f * result.reflection[0]) / result.pdf;
        }
        {
            float const r_wo_dot_h = same_side ? -wo_dot_h : wo_dot_h;

            float const n_dot_wi = ggx::Iso::refract(wo, h, n_dot_wo, n_dot_h, -wi_dot_h,
                                                     r_wo_dot_h, alpha, ior, layer, result);

            float const omf = 1.f - f;

            accum += (n_dot_wi * omf * result.reflection[0]) / result.pdf;
        }
    }

    return accum / float(num_samples);
}

float integrate_f_sd_ss(float alpha, float ior_t, float n_dot_wo, uint32_t num_samples) {
    if (alpha < Min_alpha || ior_t <= 1.f) {
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
        float3 const h = ggx::Aniso::sample(wo, float2(alpha), xi, layer, n_dot_h);

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
            float const n_dot_wi = ggx::Iso::reflect(wo, h, n_dot_wo, n_dot_h, wi_dot_h, wo_dot_h,
                                                     alpha, layer, result);

            accum += (std::min(n_dot_wi, n_dot_wo) * f * result.reflection[0]) / result.pdf;
        }
        {
            float const n_dot_wi = disney::Isotropic_no_lambert::reflect(wo, n_dot_wo, layer, alpha,
                                                                         float3(1.f), xi, result);

            accum += (n_dot_wi * result.reflection[0]) / result.pdf;
        }
        {
            float const r_wo_dot_h = same_side ? -wo_dot_h : wo_dot_h;

            float const n_dot_wi = ggx::Iso::refract(wo, h, n_dot_wo, n_dot_h, -wi_dot_h,
                                                     r_wo_dot_h, alpha, ior, layer, result);

            float const omf = 1.f - f;

            accum += (n_dot_wi * omf * result.reflection[0]) / result.pdf;
        }
    }

    return std::min(accum / float(num_samples), 1.f);
}

void make_f_ss_table(std::ostream& stream) {
    uint32_t constexpr Num_samples = 32;

    stream << "inline uint32_t constexpr E_size = " << Num_samples << ";\n\n";

    stream << "inline float constexpr E[" << Num_samples << "][" << Num_samples << "] = {\n";

    float constexpr step = 1.f / float(Num_samples - 1);

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

void make_f_s_ss_table(std::ostream& stream) {
    uint32_t constexpr Num_samples = 32;

    stream << "inline uint32_t constexpr E_s_size = " << Num_samples << ";\n\n";

    stream << "inline float constexpr E_s[" << Num_samples << "][" << Num_samples << "]["
           << Num_samples << "] = {\n";

    float constexpr step = 1.f / float(Num_samples - 1);

    float ior = 1.f;

    for (uint32_t z = 0; z < Num_samples; ++z) {
        stream << "\t// IoR " << ior << std::endl;
        stream << "\t{\n\t";

        float alpha = 0.f;
        for (uint32_t a = 0; a < Num_samples; ++a) {
            stream << "\t// alpha " << alpha << std::endl;
            stream << "\t\t{\n\t\t\t";

            float n_dot_wo = 0.f;

            for (uint32_t i = 0; i < Num_samples; ++i) {
                float const e_s = integrate_f_s_ss(alpha, ior, n_dot_wo, 1024);

                stream << e_s << "f";

                if (i < Num_samples - 1) {
                    stream << ", ";

                    if (i > 0 && 0 == ((i + 1) % 8)) {
                        stream << "\n\t\t\t";
                    }

                    n_dot_wo += step;
                } else {
                    stream << "\n\t\t}";
                }
            }

            if (a < Num_samples - 1) {
                stream << ",\n\n\t";
            } else {
                stream << "\n\t}";
            }

            alpha += step;
        }

        if (z < Num_samples - 1) {
            stream << ",\n\n";
        } else {
            stream << "\n";
        }

        ior += step;
    }

    stream << "};" << std::endl;
}

void make_f_sd_ss_table(std::ostream& stream) {
    uint32_t constexpr Num_samples = 32;

    stream << "inline uint32_t constexpr E_sd_size = " << Num_samples << ";\n\n";

    stream << "inline float constexpr E_sd[" << Num_samples << "][" << Num_samples << "]["
           << Num_samples << "] = {\n";

    float constexpr step = 1.f / float(Num_samples - 1);

    float ior = 1.f;

    for (uint32_t z = 0; z < Num_samples; ++z) {
        stream << "\t// IoR " << ior << std::endl;
        stream << "\t{\n\t";

        float alpha = 0.f;
        for (uint32_t a = 0; a < Num_samples; ++a) {
            stream << "\t// alpha " << alpha << std::endl;
            stream << "\t\t{\n\t\t\t";

            float n_dot_wo = 0.f;

            for (uint32_t i = 0; i < Num_samples; ++i) {
                float const e_s = integrate_f_sd_ss(alpha, ior, n_dot_wo, 1024);

                stream << e_s << "f";

                if (i < Num_samples - 1) {
                    stream << ", ";

                    if (i > 0 && 0 == ((i + 1) % 8)) {
                        stream << "\n\t\t\t";
                    }

                    n_dot_wo += step;
                } else {
                    stream << "\n\t\t}";
                }
            }

            if (a < Num_samples - 1) {
                stream << ",\n\n\t";
            } else {
                stream << "\n\t}";
            }

            alpha += step;
        }

        if (z < Num_samples - 1) {
            stream << ",\n\n";
        } else {
            stream << "\n";
        }

        ior += step;
    }

    stream << "};" << std::endl;
}

void integrate() {
    std::ofstream stream("../source/core/scene/material/ggx/ggx_integral.inl");

    stream << "#include <cstdint>\n\n";

    stream.precision(8);
    stream << std::fixed;

    make_f_ss_table(stream);

    stream << std::endl;

    make_f_s_ss_table(stream);

    //    stream << std::endl;

    //    make_f_sd_ss_table(stream);
}

}  // namespace scene::material::ggx
