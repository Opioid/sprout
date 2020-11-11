#ifndef SU_CORE_RENDERING_SENSOR_FILTERED_INL
#define SU_CORE_RENDERING_SENSOR_FILTERED_INL

#include "aov/value.inl"
#include "base/math/vector4.inl"
#include "filtered.hpp"
#include "sampler/camera_sample.hpp"

namespace rendering::sensor {

template <class Base, class Clamp, class F>
Filtered<Base, Clamp, F>::Filtered(Clamp const& clamp, F&& filter, int32_t filter_radius,
                                   bool adaptive)
    : Base(filter_radius, adaptive), clamp_(clamp), filter_(std::move(filter)) {}

template <class Base, class Clamp, class F>
Filtered<Base, Clamp, F>::~Filtered() = default;

template <class Base, class Clamp, class F>
void Filtered<Base, Clamp, F>::add_weighted(int2 pixel, float weight, float4 const& color,
                                            int4 const& isolated, int4 const& bounds) {
    if ((uint32_t(pixel[0] - bounds[0]) <= uint32_t(bounds[2])) &
        (uint32_t(pixel[1] - bounds[1]) <= uint32_t(bounds[3]))) {
        if ((uint32_t(pixel[0] - isolated[0]) <= uint32_t(isolated[2])) &
            (uint32_t(pixel[1] - isolated[1]) <= uint32_t(isolated[3]))) {
            Base::add_pixel(pixel, color, weight);
        } else {
            Base::add_pixel_atomic(pixel, color, weight);
        }
    }
}

template <class Base, class Clamp, class F>
void Filtered<Base, Clamp, F>::add_weighted(int2 pixel, uint32_t slot, float weight,
                                            float3 const& value, int4 const& isolated,
                                            int4 const& bounds) {
    if ((uint32_t(pixel[0] - bounds[0]) <= uint32_t(bounds[2])) &
        (uint32_t(pixel[1] - bounds[1]) <= uint32_t(bounds[3]))) {
        if ((uint32_t(pixel[0] - isolated[0]) <= uint32_t(isolated[2])) &
            (uint32_t(pixel[1] - isolated[1]) <= uint32_t(isolated[3]))) {
            Base::add_AOV(pixel, slot, value, weight);
        } else {
            Base::add_AOV_atomic(pixel, slot, value, weight);
        }
    }
}

template <class Base, class Clamp, class F>
void Filtered<Base, Clamp, F>::add_weighted(int2 pixel, float weight, float4 const& color,
                                            int4 const& bounds) {
    if ((uint32_t(pixel[0] - bounds[0]) <= uint32_t(bounds[2])) &
        (uint32_t(pixel[1] - bounds[1]) <= uint32_t(bounds[3]))) {
        Base::splat_pixel_atomic(pixel, color, weight);
    }
}

template <class Base, class Clamp, class F>
void Filtered<Base, Clamp, F>::weight_and_add(int2 pixel, float2 relative_offset,
                                              float4 const& color, int4 const& isolated,
                                              int4 const& bounds) {
    // This code assumes that (isolated_)bounds contains [x_lo, y_lo, x_hi - x_lo, y_hi - y_lo]

    if ((uint32_t(pixel[0] - bounds[0]) <= uint32_t(bounds[2])) &
        (uint32_t(pixel[1] - bounds[1]) <= uint32_t(bounds[3]))) {
        float const weight = filter_.evaluate(relative_offset);

        if ((uint32_t(pixel[0] - isolated[0]) <= uint32_t(isolated[2])) &
            (uint32_t(pixel[1] - isolated[1]) <= uint32_t(isolated[3]))) {
            Base::add_pixel(pixel, color, weight);
        } else {
            Base::add_pixel_atomic(pixel, color, weight);
        }
    }
}

template <class Base, class Clamp, class F>
void Filtered<Base, Clamp, F>::weight_and_add(int2 pixel, float2 relative_offset,
                                              float4 const& color, int4 const& bounds) {
    // This code assumes that (isolated_)bounds contains [x_lo, y_lo, x_hi - x_lo, y_hi - y_lo]

    if ((uint32_t(pixel[0] - bounds[0]) <= uint32_t(bounds[2])) &
        (uint32_t(pixel[1] - bounds[1]) <= uint32_t(bounds[3]))) {
        float const weight = filter_.evaluate(relative_offset);

        Base::add_pixel_atomic(pixel, color, weight);
    }
}

template <class Base, class Clamp, class F>
Filtered_1p0<Base, Clamp, F>::Filtered_1p0(Clamp const& clamp, F&& filter, bool adaptive)
    : Filtered_base(clamp, std::move(filter), 1, adaptive) {}

template <class Base, class Clamp, class F>
float4 Filtered_1p0<Base, Clamp, F>::add_sample(Sample const& sample, float4 const& color,
                                                aov::Value const* aov, int4 const& isolated,
                                                int2 offset, int4 const& bounds) {
    float4 const clamped = Filtered_base::clamp_.clamp(color);

    int32_t const x = offset[0] + sample.pixel[0];
    int32_t const y = offset[1] + sample.pixel[1];

    float const ox = sample.pixel_uv[0] - 0.5f;
    float const oy = sample.pixel_uv[1] - 0.5f;

    float const wx0 = Filtered_base::filter_.evaluate(ox + 1.f);
    float const wx1 = Filtered_base::filter_.evaluate(ox);
    float const wx2 = Filtered_base::filter_.evaluate(ox - 1.f);

    float const wy0 = Filtered_base::filter_.evaluate(oy + 1.f);
    float const wy1 = Filtered_base::filter_.evaluate(oy);
    float const wy2 = Filtered_base::filter_.evaluate(oy - 1.f);

    // 1. row
    Filtered_base::add_weighted(int2(x - 1, y - 1), wx0 * wy0, clamped, isolated, bounds);
    Filtered_base::add_weighted(int2(x, y - 1), wx1 * wy0, clamped, isolated, bounds);
    Filtered_base::add_weighted(int2(x + 1, y - 1), wx2 * wy0, clamped, isolated, bounds);

    // 2. row
    Filtered_base::add_weighted(int2(x - 1, y), wx0 * wy1, clamped, isolated, bounds);
    Filtered_base::add_weighted(int2(x, y), wx1 * wy1, clamped, isolated, bounds);
    Filtered_base::add_weighted(int2(x + 1, y), wx2 * wy1, clamped, isolated, bounds);

    // 3. row
    Filtered_base::add_weighted(int2(x - 1, y + 1), wx0 * wy2, clamped, isolated, bounds);
    Filtered_base::add_weighted(int2(x, y + 1), wx1 * wy2, clamped, isolated, bounds);
    Filtered_base::add_weighted(int2(x + 1, y + 1), wx2 * wy2, clamped, isolated, bounds);

    if (aov) {
        for (uint32_t i = 0, len = aov->num_slots(); i < len; ++i) {
            auto const v = aov->value(i);

            Filtered_base::add_weighted(int2(x, y), i, 1.f, v, isolated, bounds);
        }
    }

    return clamped;
}

template <class Base, class Clamp, class F>
void Filtered_1p0<Base, Clamp, F>::splat_sample(Sample_to const& sample, float4 const& color,
                                                int2 offset, int4 const& bounds) {
    float4 const clamped = Filtered_base::clamp_.clamp(color);

    int32_t const x = offset[0] + sample.pixel[0];
    int32_t const y = offset[1] + sample.pixel[1];

    float const ox = sample.pixel_uv[0] - 0.5f;
    float const oy = sample.pixel_uv[1] - 0.5f;

    float const wx0 = Filtered_base::filter_.evaluate(ox + 1.f);
    float const wx1 = Filtered_base::filter_.evaluate(ox);
    float const wx2 = Filtered_base::filter_.evaluate(ox - 1.f);

    float const wy0 = Filtered_base::filter_.evaluate(oy + 1.f);
    float const wy1 = Filtered_base::filter_.evaluate(oy);
    float const wy2 = Filtered_base::filter_.evaluate(oy - 1.f);

    // 1. row
    Filtered_base::add_weighted(int2(x - 1, y - 1), wx0 * wy0, clamped, bounds);
    Filtered_base::add_weighted(int2(x, y - 1), wx1 * wy0, clamped, bounds);
    Filtered_base::add_weighted(int2(x + 1, y - 1), wx2 * wy0, clamped, bounds);

    // 2. row
    Filtered_base::add_weighted(int2(x - 1, y), wx0 * wy1, clamped, bounds);
    Filtered_base::add_weighted(int2(x, y), wx1 * wy1, clamped, bounds);
    Filtered_base::add_weighted(int2(x + 1, y), wx2 * wy1, clamped, bounds);

    // 3. row
    Filtered_base::add_weighted(int2(x - 1, y + 1), wx0 * wy2, clamped, bounds);
    Filtered_base::add_weighted(int2(x, y + 1), wx1 * wy2, clamped, bounds);
    Filtered_base::add_weighted(int2(x + 1, y + 1), wx2 * wy2, clamped, bounds);
}

template <class Base, class Clamp, class F>
Filtered_2p0<Base, Clamp, F>::Filtered_2p0(Clamp const& clamp, F&& filter, bool adaptive)
    : Filtered_base(clamp, std::move(filter), 2, adaptive) {}

template <class Base, class Clamp, class F>
float4 Filtered_2p0<Base, Clamp, F>::add_sample(Sample const& sample, float4 const& color,
                                                aov::Value const* aov, int4 const& isolated,
                                                int2 offset, int4 const& bounds) {
    float4 const clamped = Filtered_base::clamp_.clamp(color);

    int32_t const x = offset[0] + sample.pixel[0];
    int32_t const y = offset[1] + sample.pixel[1];

    float const ox = sample.pixel_uv[0] - 0.5f;
    float const oy = sample.pixel_uv[1] - 0.5f;

    float const wx0 = Filtered_base::filter_.evaluate(ox + 2.f);
    float const wx1 = Filtered_base::filter_.evaluate(ox + 1.f);
    float const wx2 = Filtered_base::filter_.evaluate(ox);
    float const wx3 = Filtered_base::filter_.evaluate(ox - 1.f);
    float const wx4 = Filtered_base::filter_.evaluate(ox - 2.f);

    float const wy0 = Filtered_base::filter_.evaluate(oy + 2.f);
    float const wy1 = Filtered_base::filter_.evaluate(oy + 1.f);
    float const wy2 = Filtered_base::filter_.evaluate(oy);
    float const wy3 = Filtered_base::filter_.evaluate(oy - 1.f);
    float const wy4 = Filtered_base::filter_.evaluate(oy - 2.f);

    // 1. row
    Filtered_base::add_weighted(int2(x - 2, y - 2), wx0 * wy0, clamped, isolated, bounds);
    Filtered_base::add_weighted(int2(x - 1, y - 2), wx1 * wy0, clamped, isolated, bounds);
    Filtered_base::add_weighted(int2(x, y - 2), wx2 * wy0, clamped, isolated, bounds);
    Filtered_base::add_weighted(int2(x + 1, y - 2), wx3 * wy0, clamped, isolated, bounds);
    Filtered_base::add_weighted(int2(x + 2, y - 2), wx4 * wy0, clamped, isolated, bounds);

    // 2. row
    Filtered_base::add_weighted(int2(x - 2, y - 1), wx0 * wy1, clamped, isolated, bounds);
    Filtered_base::add_weighted(int2(x - 1, y - 1), wx1 * wy1, clamped, isolated, bounds);
    Filtered_base::add_weighted(int2(x, y - 1), wx2 * wy1, clamped, isolated, bounds);
    Filtered_base::add_weighted(int2(x + 1, y - 1), wx3 * wy1, clamped, isolated, bounds);
    Filtered_base::add_weighted(int2(x + 2, y - 1), wx4 * wy1, clamped, isolated, bounds);

    // 3. row
    Filtered_base::add_weighted(int2(x - 2, y), wx0 * wy2, clamped, isolated, bounds);
    Filtered_base::add_weighted(int2(x - 1, y), wx1 * wy2, clamped, isolated, bounds);
    Filtered_base::add_weighted(int2(x, y), wx2 * wy2, clamped, isolated, bounds);
    Filtered_base::add_weighted(int2(x + 1, y), wx3 * wy2, clamped, isolated, bounds);
    Filtered_base::add_weighted(int2(x + 2, y), wx4 * wy2, clamped, isolated, bounds);

    // 4. row
    Filtered_base::add_weighted(int2(x - 2, y + 1), wx0 * wy3, clamped, isolated, bounds);
    Filtered_base::add_weighted(int2(x - 1, y + 1), wx1 * wy3, clamped, isolated, bounds);
    Filtered_base::add_weighted(int2(x, y + 1), wx2 * wy3, clamped, isolated, bounds);
    Filtered_base::add_weighted(int2(x + 1, y + 1), wx3 * wy3, clamped, isolated, bounds);
    Filtered_base::add_weighted(int2(x + 2, y + 1), wx4 * wy3, clamped, isolated, bounds);

    // 5. row
    Filtered_base::add_weighted(int2(x - 2, y + 2), wx0 * wy4, clamped, isolated, bounds);
    Filtered_base::add_weighted(int2(x - 1, y + 2), wx1 * wy4, clamped, isolated, bounds);
    Filtered_base::add_weighted(int2(x, y + 2), wx2 * wy4, clamped, isolated, bounds);
    Filtered_base::add_weighted(int2(x + 1, y + 2), wx3 * wy4, clamped, isolated, bounds);
    Filtered_base::add_weighted(int2(x + 2, y + 2), wx4 * wy4, clamped, isolated, bounds);

    if (aov) {
        for (uint32_t i = 0, len = aov->num_slots(); i < len; ++i) {
            auto const v = aov->value(i);

            Filtered_base::add_weighted(int2(x, y), i, 1.f, v, isolated, bounds);
        }
    }

    return clamped;
}

template <class Base, class Clamp, class F>
void Filtered_2p0<Base, Clamp, F>::splat_sample(Sample_to const& sample, float4 const& color,
                                                int2 offset, int4 const& bounds) {
    float4 const clamped = Filtered_base::clamp_.clamp(color);

    int32_t const x = offset[0] + sample.pixel[0];
    int32_t const y = offset[1] + sample.pixel[1];

    float const ox = sample.pixel_uv[0] - 0.5f;
    float const oy = sample.pixel_uv[1] - 0.5f;

    float const wx0 = Filtered_base::filter_.evaluate(ox + 2.f);
    float const wx1 = Filtered_base::filter_.evaluate(ox + 1.f);
    float const wx2 = Filtered_base::filter_.evaluate(ox);
    float const wx3 = Filtered_base::filter_.evaluate(ox - 1.f);
    float const wx4 = Filtered_base::filter_.evaluate(ox - 2.f);

    float const wy0 = Filtered_base::filter_.evaluate(oy + 2.f);
    float const wy1 = Filtered_base::filter_.evaluate(oy + 1.f);
    float const wy2 = Filtered_base::filter_.evaluate(oy);
    float const wy3 = Filtered_base::filter_.evaluate(oy - 1.f);
    float const wy4 = Filtered_base::filter_.evaluate(oy - 2.f);

    // 1. row
    Filtered_base::add_weighted(int2(x - 2, y - 2), wx0 * wy0, clamped, bounds);
    Filtered_base::add_weighted(int2(x - 1, y - 2), wx1 * wy0, clamped, bounds);
    Filtered_base::add_weighted(int2(x, y - 2), wx2 * wy0, clamped, bounds);
    Filtered_base::add_weighted(int2(x + 1, y - 2), wx3 * wy0, clamped, bounds);
    Filtered_base::add_weighted(int2(x + 2, y - 2), wx4 * wy0, clamped, bounds);

    // 2. row
    Filtered_base::add_weighted(int2(x - 2, y - 1), wx0 * wy1, clamped, bounds);
    Filtered_base::add_weighted(int2(x - 1, y - 1), wx1 * wy1, clamped, bounds);
    Filtered_base::add_weighted(int2(x, y - 1), wx2 * wy1, clamped, bounds);
    Filtered_base::add_weighted(int2(x + 1, y - 1), wx3 * wy1, clamped, bounds);
    Filtered_base::add_weighted(int2(x + 2, y - 1), wx4 * wy1, clamped, bounds);

    // 3. row
    Filtered_base::add_weighted(int2(x - 2, y), wx0 * wy2, clamped, bounds);
    Filtered_base::add_weighted(int2(x - 1, y), wx1 * wy2, clamped, bounds);
    Filtered_base::add_weighted(int2(x, y), wx2 * wy2, clamped, bounds);
    Filtered_base::add_weighted(int2(x + 1, y), wx3 * wy2, clamped, bounds);
    Filtered_base::add_weighted(int2(x + 2, y), wx4 * wy2, clamped, bounds);

    // 4. row
    Filtered_base::add_weighted(int2(x - 2, y + 1), wx0 * wy3, clamped, bounds);
    Filtered_base::add_weighted(int2(x - 1, y + 1), wx1 * wy3, clamped, bounds);
    Filtered_base::add_weighted(int2(x, y + 1), wx2 * wy3, clamped, bounds);
    Filtered_base::add_weighted(int2(x + 1, y + 1), wx3 * wy3, clamped, bounds);
    Filtered_base::add_weighted(int2(x + 2, y + 1), wx4 * wy3, clamped, bounds);

    // 5. row
    Filtered_base::add_weighted(int2(x - 2, y + 2), wx0 * wy4, clamped, bounds);
    Filtered_base::add_weighted(int2(x - 1, y + 2), wx1 * wy4, clamped, bounds);
    Filtered_base::add_weighted(int2(x, y + 2), wx2 * wy4, clamped, bounds);
    Filtered_base::add_weighted(int2(x + 1, y + 2), wx3 * wy4, clamped, bounds);
    Filtered_base::add_weighted(int2(x + 2, y + 2), wx4 * wy4, clamped, bounds);
}

template <class Base, class Clamp, class F>
Filtered_inf<Base, Clamp, F>::Filtered_inf(Clamp const& clamp, F&& filter, float filter_radius,
                                           bool adaptive)
    : Filtered_base(clamp, std::move(filter), int32_t(std::ceil(filter_radius)), adaptive),
      filter_radius_(filter_radius) {}

template <class Base, class Clamp, class F>
float4 Filtered_inf<Base, Clamp, F>::add_sample(Sample const& sample, float4 const& color,
                                                aov::Value const* aov, int4 const& isolated,
                                                int2 offset, int4 const& bounds) {
    float4 const clamped = Filtered_base::clamp_.clamp(color);

    int32_t const px = offset[0] + sample.pixel[0];
    int32_t const py = offset[1] + sample.pixel[1];

    int32_t const r  = Filtered_base::filter_radius_int();
    float const   rf = filter_radius_;

    for (int32_t ky = -r; ky <= r; ++ky) {
        for (int32_t kx = -r; kx <= r; ++kx) {
            int2 const pixel(px + kx, py + ky);

            float2 const ro = sample.pixel_uv - 0.5f - float2(kx, ky);

            if ((ro[0] < rf) & (ro[1] < rf)) {
                Filtered_base::weight_and_add(pixel, ro, clamped, isolated, bounds);
            }
        }
    }

    if (aov) {
        for (uint32_t i = 0, len = aov->num_slots(); i < len; ++i) {
            auto const v = aov->value(i);

            Filtered_base::add_weighted(int2(px, py), i, 1.f, v, isolated, bounds);
        }
    }

    return clamped;
}

template <class Base, class Clamp, class F>
void Filtered_inf<Base, Clamp, F>::splat_sample(Sample_to const& sample, float4 const& color,
                                                int2 offset, int4 const& bounds) {
    float4 const clamped = Filtered_base::clamp_.clamp(color);

    int32_t const px = offset[0] + sample.pixel[0];
    int32_t const py = offset[1] + sample.pixel[1];

    int32_t const r  = Filtered_base::filter_radius_int();
    float const   rf = filter_radius_;

    for (int32_t ky = -r; ky <= r; ++ky) {
        for (int32_t kx = -r; kx <= r; ++kx) {
            int2 const pixel(px + kx, py + ky);

            float2 const ro = sample.pixel_uv - 0.5f - float2(kx, ky);

            if ((ro[0] < rf) & (ro[1] < rf)) {
                Filtered_base::weight_and_add(pixel, ro, clamped, bounds);
            }
        }
    }
}

}  // namespace rendering::sensor

#endif
