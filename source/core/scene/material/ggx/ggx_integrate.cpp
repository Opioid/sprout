#include "ggx_integrate.hpp"
#include "base/math/sample_distribution.inl"
#include "ggx.inl"

#include <fstream>
#include <iostream>

// https://blog.selfshadow.com/2018/06/04/multi-faceted-part-2/
// https://google.github.io/filament/Filament.html#materialsystem/improvingthebrdfs/energylossinspecularreflectance

namespace scene::material::ggx {

float3 importance_sample_GGX(float2 xi, float a, float3 const& n) {
    float const phi = 2.f * Pi * xi[0];

    float const cos_theta = std::sqrt((1.f - xi[1]) / (1.f + (a * a - 1.f) * xi[1]));

    float const sin_theta = std::sqrt(1.f - cos_theta * cos_theta);

    float3 const h(sin_theta * cos(phi), sin_theta * sin(phi), cos_theta);

    float3 const up = std::abs(n[2]) < 0.999f ? float3(0.f, 0, 1.f) : float3(1.f, 0.f, 0.f);

    float3 const tangent_x = normalize(cross(up, n));

    float3 const tangent_y = cross(n, tangent_x);

    // Tangent to world space
    return h[0] * tangent_x + h[1] * tangent_y + h[2] * n;
}

float3 sample(float2 xi, float alpha, float3 const& n, float3 const& wo) noexcept {
    //   float3 const lwo = layer.world_to_tangent(wo);

    // stretch view
    float3 const v = normalize(float3(alpha * wo[0], alpha * wo[1], wo[2]));

    // orthonormal basis
    float3 const cross_v_z = float3(v[1], -v[0], 0.f);  // == cross(v, [0, 0, 1])

    float3 const t1 = (v[2] < 0.9999f) ? normalize(cross_v_z) : float3(1.f, 0.f, 0.f);
    float3 const t2 = float3(t1[1] * v[2], -t1[0] * v[2], t1[0] * v[1] - t1[1] * v[0]);

    // sample point with polar coordinates (r, phi)
    float const a   = 1.f / (1.f + v[2]);
    float const r   = std::sqrt(xi[0]);
    float const phi = (xi[1] < a) ? (xi[1] / a * Pi) : (Pi + (xi[1] - a) / (1.f - a) * Pi);

    auto const [sin_phi, cos_phi] = sincos(phi);

    float const p1 = r * cos_phi;
    float const p2 = r * sin_phi * ((xi[1] < a) ? 1.f : v[2]);

    // compute normal
    float3 m = p1 * t1 + p2 * t2 + std::sqrt(std::max(1.f - p1 * p1 - p2 * p2, 0.f)) * v;

    // unstretch
    m = normalize(float3(alpha * m[0], alpha * m[1], std::max(m[2], 0.f)));

    return m;

    //    float3 const h = layer.tangent_to_world(m);

    //   return h;
}

float f_ss(float n_dot_wi, float n_dot_wo, float wo_dot_h, float n_dot_h, float alpha) noexcept {
    float const alpha2 = alpha * alpha;

    float const  d = distribution_isotropic(n_dot_h, alpha2);
    float2 const g = optimized_masking_shadowing_and_g1_wo(n_dot_wi, n_dot_wo, alpha2);
    //   float const f = fresnel::schlick(wo_dot_h, 1.f);

    return n_dot_wi * d * g[0];  // * f;
}

float integrate_f_ss(float alpha, float n_dot_wo, uint32_t num_samples) {
    const float3 n(0.f, 0.f, 1.f);

    n_dot_wo = std::max(n_dot_wo, Dot_min);

    // (sin, 0, cos)
    float3 const wo(std::sqrt(1.f - n_dot_wo * n_dot_wo), 0.f, n_dot_wo);

    float f = 0.f;

    for (uint32_t i = 0; i < num_samples; ++i) {
        float2 const xi = hammersley(i, num_samples, 0);

        float3 const h = importance_sample_GGX(xi, alpha, n);

        // float3 const h = sample(xi, alpha, n, wo);

        float3 const wi = normalize(2.f * dot(wo, h) * h - wo);

        if (wi[2] < 0.f) {
            continue;
        }

        float const n_dot_wi = clamp(wi[2]);
        float const n_dot_h  = saturate(h[2]);
        float const wo_dot_h = saturate(dot(wo, h));

        f += f_ss(n_dot_wi, n_dot_wo, wo_dot_h, n_dot_h, alpha);
    }

    return f / static_cast<float>(num_samples);
}

void integrate() noexcept {
    std::cout << "random integrate: " << integrate_f_ss(1.f, 0.5f, 1024) << std::endl;

    uint32_t constexpr Num_samples = 16;

    std::ofstream stream("../source/core/scene/material/ggx/ggx_energy_preservation.inl");

    stream << "uint32_t constexpr E_max = " << Num_samples - 1 << ";\n\n";

    stream << "float constexpr E_scale = " << Num_samples << ".f;\n\n";

    stream << "float constexpr E[" << Num_samples << "][" << Num_samples << "] = {\n";

    float constexpr step = 1.f / static_cast<float>(Num_samples);

    float alpha = 0.5f * step;

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
}

}  // namespace scene::material::ggx
