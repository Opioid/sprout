#include "piecewise.hpp"
#include "base/math/exp.hpp"
#include "base/math/vector4.inl"
#include "image/typed_image.hpp"

namespace rendering::postprocessor::tonemapping {

struct R2 {
    float a;
    float b;
};

static R2 solve_ab(float x0, float y0, float m) {
    float const b    = (m * x0) / y0;
    float const ln_a = std::log(y0) - b * std::log(x0);

    return {ln_a, b};
}

static R2 as_slope_intercept(float x0, float x1, float y0, float y1) {
    float const dy = y1 - y0;
    float const dx = x1 - x0;

    float const m = (0.f == dx) ? 1.f : dy / dx;

    float const b = y0 - x0 * m;

    return {m, b};
}

struct Parameters {
    float x0;
    float y0;
    float x1;
    float y1;
    float w;

    float overshoot_x;
    float overshoot_y;
};

static void CalcDirectParamsFromUser(Parameters& params, float toe_strength, float toe_length,
                                     float shoulder_strength, float shoulder_length,
                                     float shoulder_angle) {
    // This is not actually the display gamma. It's just a UI space to avoid having to
    // enter small numbers for the input.
    static float constexpr perceptualGamma = 2.2f;

    // constraints

    toe_length      = std::pow(saturate(toe_length), perceptualGamma);
    toe_strength    = saturate(toe_strength);
    shoulder_angle  = saturate(shoulder_angle);
    shoulder_length = std::max(1e-5f, saturate(shoulder_length));

    shoulder_strength = std::max(0.0f, shoulder_strength);

    // apply base params

    // toe goes from 0 to 0.5
    float const x0 = toe_length * .5f;
    float const y0 = (1.0f - toe_strength) * x0;  // lerp from 0 to x0

    float const remaining_y = 1.0f - y0;

    float const initial_w = x0 + remaining_y;

    float const y1_offset = (1.0f - shoulder_length) * remaining_y;
    float const x1        = x0 + y1_offset;
    float const y1        = y0 + y1_offset;

    // filmic shoulder strength is in F stops
    float const extraW = std::exp2(shoulder_strength) - 1.0f;

    float const w = initial_w + extraW;

    params.x0 = x0;
    params.y0 = y0;
    params.x1 = x1;
    params.y1 = y1;
    params.w  = w;

    params.overshoot_x = (params.w * 2.0f) * shoulder_angle * shoulder_strength;
    params.overshoot_y = 0.5f * shoulder_angle * shoulder_strength;
}

Piecewise::Piecewise(float exposure, float toe_strength, float toe_length, float shoulder_strength,
                     float shoulder_length, float shoulder_angle)
    : Tonemapper(exposure) {
    Parameters params;

    CalcDirectParamsFromUser(params, toe_strength, toe_length, shoulder_strength, shoulder_length,
                             shoulder_angle);

    float const w = params.w;

    m_W    = w;
    m_invW = 1.f / w;

    // normalize params to 1.0 range
    params.w = 1.f;
    params.x0 /= w;
    params.x1 /= w;
    params.overshoot_x /= w;

    float toe_m      = 0.f;
    float shoulder_m = 0.f;

    {
        auto const [m, b] = as_slope_intercept(params.x0, params.x1, params.y0, params.y1);

        Piecewise::CurveSegment mid;
        mid.offset_x = -(b / m);
        mid.offset_y = 0.f;
        mid.scale_x  = 1.f;
        mid.scale_y  = 1.f;
        mid.ln_a     = std::log(m);
        mid.b        = 1.f;

        segments_[1] = mid;

        toe_m      = m;
        shoulder_m = m;
    }

    m_x0 = params.x0;
    m_x1 = params.x1;
    m_y0 = params.y0;
    m_y1 = params.y1;

    // toe section
    {
        auto const [ln_a, b] = solve_ab(params.x0, params.y0, toe_m);

        Piecewise::CurveSegment toe;
        toe.offset_x = 0.f;
        toe.offset_y = 0.f;
        toe.scale_x  = 1.f;
        toe.scale_y  = 1.f;
        toe.ln_a     = ln_a;
        toe.b        = b;

        segments_[0] = toe;
    }

    // shoulder section
    {
        // use the simple version that is usually too flat

        float const x0 = (1.f + params.overshoot_x) - params.x1;
        float const y0 = (1.f + params.overshoot_y) - params.y1;

        auto const [ln_a, b] = solve_ab(x0, y0, shoulder_m);

        Piecewise::CurveSegment shoulder;
        shoulder.offset_x = 1.f + params.overshoot_x;
        shoulder.offset_y = 1.f + params.overshoot_y;
        shoulder.scale_x  = -1.f;
        shoulder.scale_y  = -1.f;
        shoulder.ln_a     = ln_a;
        shoulder.b        = b;

        segments_[2] = shoulder;
    }

    // Normalize so that we hit 1.0 at our white point. We wouldn't have do this if we
    // skipped the overshoot part.
    {
        // evaluate shoulder at the end of the curve
        float const scale     = segments_[2].eval(1.f);
        float const inv_scale = 1.f / scale;

        segments_[0].offset_y *= inv_scale;
        segments_[0].scale_y *= inv_scale;

        segments_[1].offset_y *= inv_scale;
        segments_[1].scale_y *= inv_scale;

        segments_[2].offset_y *= inv_scale;
        segments_[2].scale_y *= inv_scale;
    }
}

void Piecewise::apply(uint32_t /*id*/, uint32_t /*pass*/, int32_t begin, int32_t end,
                      image::Float4 const& source, image::Float4& destination) {
    float const factor = exposure_factor_;

    for (int32_t i = begin; i < end; ++i) {
        float4 const& color = source.at(i);

        destination.store(i, float4(eval(factor * color[0]), eval(factor * color[1]),
                                    eval(factor * color[2]), color[3]));
    }
}

float Piecewise::CurveSegment::eval(float x) const {
    // log(0) is undefined but our function should evaluate to 0.
    // There are better ways to handle this, but it's doing it the slow way here for clarity.

    float const x0 = (x - offset_x) * scale_x;
    float const y0 = x0 > 0.f ? math::exp(ln_a + b * math::log(x0)) : 0.f;

    return y0 * scale_y + offset_y;
}

float Piecewise::eval(float srcX) const {
    float normX = srcX * m_invW;

    int32_t const index = (normX < m_x0) ? 0 : ((normX < m_x1) ? 1 : 2);

    CurveSegment const segment = segments_[index];

    float ret = segment.eval(normX);

    return ret;
}

}  // namespace rendering::postprocessor::tonemapping
