#ifndef SU_CORE_RENDERING_SENSOR_FILTERED_INL
#define SU_CORE_RENDERING_SENSOR_FILTERED_INL

#include "base/math/vector4.inl"
#include "filter/sensor_filter.hpp"
#include "filtered.hpp"
#include "sampler/camera_sample.hpp"

namespace rendering::sensor {

template <class Base, class Clamp>
Filtered<Base, Clamp>::Filtered(int2 dimensions, float exposure, const Clamp& clamp,
                                filter::Filter const* filter) noexcept
    : Base(dimensions, exposure), clamp_(clamp), filter_(filter) {}

template <class Base, class Clamp>
Filtered<Base, Clamp>::Filtered(int2 dimensions, float exposure, Texture_ptr const& backplate,
                                const Clamp& clamp, filter::Filter const* filter) noexcept
    : Base(dimensions, exposure, backplate), clamp_(clamp), filter_(filter) {}

template <class Base, class Clamp>
Filtered<Base, Clamp>::~Filtered() noexcept {
    delete filter_;
}

template <class Base, class Clamp>
int32_t Filtered<Base, Clamp>::filter_radius_int() const noexcept {
    return static_cast<int32_t>(std::ceil(filter_->radius()));
}

template <class Base, class Clamp>
int4 Filtered<Base, Clamp>::isolated_tile(int4 const& tile) const noexcept {
    int32_t const r = filter_radius_int();
    return tile + int4(r, r, -r, -r);
}

template <class Base, class Clamp>
void Filtered<Base, Clamp>::add_weighted(int2 pixel, float weight, float4 const& color,
                                         int4 const& isolated_bounds, int4 const& bounds) noexcept {
    if (static_cast<uint32_t>(pixel[0] - bounds[0]) <= static_cast<uint32_t>(bounds[2]) &&
        static_cast<uint32_t>(pixel[1] - bounds[1]) <= static_cast<uint32_t>(bounds[3])) {
        if (static_cast<uint32_t>(pixel[0] - isolated_bounds[0]) <=
                static_cast<uint32_t>(isolated_bounds[2]) &&
            static_cast<uint32_t>(pixel[1] - isolated_bounds[1]) <=
                static_cast<uint32_t>(isolated_bounds[3])) {
            Base::add_pixel(pixel, color, weight);
        } else {
            Base::add_pixel_atomic(pixel, color, weight);
        }
    }
}

template <class Base, class Clamp>
void Filtered<Base, Clamp>::weight_and_add(int2 pixel, float2 relative_offset, float4 const& color,
                                           int4 const& isolated_bounds,
                                           int4 const& bounds) noexcept {
    // This code assumes that (isolated_)bounds contains [x_lo, y_lo, x_hi - x_lo, y_hi - y_lo]

    if (static_cast<uint32_t>(pixel[0] - bounds[0]) <= static_cast<uint32_t>(bounds[2]) &&
        static_cast<uint32_t>(pixel[1] - bounds[1]) <= static_cast<uint32_t>(bounds[3])) {
        float const weight = filter_->evaluate(relative_offset);

        if (static_cast<uint32_t>(pixel[0] - isolated_bounds[0]) <=
                static_cast<uint32_t>(isolated_bounds[2]) &&
            static_cast<uint32_t>(pixel[1] - isolated_bounds[1]) <=
                static_cast<uint32_t>(isolated_bounds[3])) {
            Base::add_pixel(pixel, color, weight);
        } else {
            Base::add_pixel_atomic(pixel, color, weight);
        }
    }
}

template <class Base, class Clamp>
Filtered_1p0<Base, Clamp>::Filtered_1p0(int2 dimensions, float exposure, const Clamp& clamp,
                                        filter::Filter const* filter) noexcept
    : Filtered<Base, Clamp>(dimensions, exposure, clamp, filter) {}

template <class Base, class Clamp>
Filtered_1p0<Base, Clamp>::Filtered_1p0(int2 dimensions, float exposure,
                                        Texture_ptr const& backplate, const Clamp& clamp,
                                        filter::Filter const* filter) noexcept
    : Filtered<Base, Clamp>(dimensions, exposure, backplate, clamp, filter) {}

template <class Base, class Clamp>
void Filtered_1p0<Base, Clamp>::add_sample(sampler::Camera_sample const& sample,
                                           float4 const& color, int4 const& isolated,
                                           int4 const& bounds) noexcept {
    float4 const clamped = Filtered<Base, Clamp>::clamp_.clamp(color);

    int32_t const x = bounds[0] + sample.pixel[0];
    int32_t const y = bounds[1] + sample.pixel[1];

    float const ox = sample.pixel_uv[0] - 0.5f;
    float const oy = sample.pixel_uv[1] - 0.5f;

    float const wx0 = Filtered<Base, Clamp>::filter_->evaluate(ox + 1.f);
    float const wx1 = Filtered<Base, Clamp>::filter_->evaluate(ox);
    float const wx2 = Filtered<Base, Clamp>::filter_->evaluate(ox - 1.f);

    float const wy0 = Filtered<Base, Clamp>::filter_->evaluate(oy + 1.f);
    float const wy1 = Filtered<Base, Clamp>::filter_->evaluate(oy);
    float const wy2 = Filtered<Base, Clamp>::filter_->evaluate(oy - 1.f);

    // 1. row
    Filtered<Base, Clamp>::add_weighted(int2(x - 1, y - 1), wx0 * wy0, clamped, isolated, bounds);
    Filtered<Base, Clamp>::add_weighted(int2(x, y - 1), wx1 * wy0, clamped, isolated, bounds);
    Filtered<Base, Clamp>::add_weighted(int2(x + 1, y - 1), wx2 * wy0, clamped, isolated, bounds);

    // 2. row
    Filtered<Base, Clamp>::add_weighted(int2(x - 1, y), wx0 * wy1, clamped, isolated, bounds);
    Filtered<Base, Clamp>::add_weighted(int2(x, y), wx1 * wy1, clamped, isolated, bounds);
    Filtered<Base, Clamp>::add_weighted(int2(x + 1, y), wx2 * wy1, clamped, isolated, bounds);

    // 3. row
    Filtered<Base, Clamp>::add_weighted(int2(x - 1, y + 1), wx0 * wy2, clamped, isolated, bounds);
    Filtered<Base, Clamp>::add_weighted(int2(x, y + 1), wx1 * wy2, clamped, isolated, bounds);
    Filtered<Base, Clamp>::add_weighted(int2(x + 1, y + 1), wx2 * wy2, clamped, isolated, bounds);
}

template <class Base, class Clamp>
Filtered_2p0<Base, Clamp>::Filtered_2p0(int2 dimensions, float exposure, const Clamp& clamp,
                                        filter::Filter const* filter) noexcept
    : Filtered<Base, Clamp>(dimensions, exposure, clamp, filter) {}

template <class Base, class Clamp>
Filtered_2p0<Base, Clamp>::Filtered_2p0(int2 dimensions, float exposure,
                                        Texture_ptr const& backplate, const Clamp& clamp,
                                        filter::Filter const* filter) noexcept
    : Filtered<Base, Clamp>(dimensions, exposure, backplate, clamp, filter) {}

template <class Base, class Clamp>
void Filtered_2p0<Base, Clamp>::add_sample(sampler::Camera_sample const& sample,
                                           float4 const& color, int4 const& isolated,
                                           int4 const& bounds) noexcept {
    float4 const clamped = Filtered<Base, Clamp>::clamp_.clamp(color);

    int32_t const x = bounds[0] + sample.pixel[0];
    int32_t const y = bounds[1] + sample.pixel[1];

    float const ox = sample.pixel_uv[0] - 0.5f;
    float const oy = sample.pixel_uv[1] - 0.5f;

    float const wx0 = Filtered<Base, Clamp>::filter_->evaluate(ox + 2.f);
    float const wx1 = Filtered<Base, Clamp>::filter_->evaluate(ox + 1.f);
    float const wx2 = Filtered<Base, Clamp>::filter_->evaluate(ox);
    float const wx3 = Filtered<Base, Clamp>::filter_->evaluate(ox - 1.f);
    float const wx4 = Filtered<Base, Clamp>::filter_->evaluate(ox - 2.f);

    float const wy0 = Filtered<Base, Clamp>::filter_->evaluate(oy + 2.f);
    float const wy1 = Filtered<Base, Clamp>::filter_->evaluate(oy + 1.f);
    float const wy2 = Filtered<Base, Clamp>::filter_->evaluate(oy);
    float const wy3 = Filtered<Base, Clamp>::filter_->evaluate(oy - 1.f);
    float const wy4 = Filtered<Base, Clamp>::filter_->evaluate(oy - 2.f);

    // 1. row
    Filtered<Base, Clamp>::add_weighted(int2(x - 2, y - 2), wx0 * wy0, clamped, isolated, bounds);
    Filtered<Base, Clamp>::add_weighted(int2(x - 1, y - 2), wx1 * wy0, clamped, isolated, bounds);
    Filtered<Base, Clamp>::add_weighted(int2(x, y - 2), wx2 * wy0, clamped, isolated, bounds);
    Filtered<Base, Clamp>::add_weighted(int2(x + 1, y - 2), wx3 * wy0, clamped, isolated, bounds);
    Filtered<Base, Clamp>::add_weighted(int2(x + 2, y - 2), wx4 * wy0, clamped, isolated, bounds);

    // 2. row
    Filtered<Base, Clamp>::add_weighted(int2(x - 2, y - 1), wx0 * wy1, clamped, isolated, bounds);
    Filtered<Base, Clamp>::add_weighted(int2(x - 1, y - 1), wx1 * wy1, clamped, isolated, bounds);
    Filtered<Base, Clamp>::add_weighted(int2(x, y - 1), wx2 * wy1, clamped, isolated, bounds);
    Filtered<Base, Clamp>::add_weighted(int2(x + 1, y - 1), wx3 * wy1, clamped, isolated, bounds);
    Filtered<Base, Clamp>::add_weighted(int2(x + 2, y - 1), wx4 * wy1, clamped, isolated, bounds);

    // 3. row
    Filtered<Base, Clamp>::add_weighted(int2(x - 2, y), wx0 * wy2, clamped, isolated, bounds);
    Filtered<Base, Clamp>::add_weighted(int2(x - 1, y), wx1 * wy2, clamped, isolated, bounds);
    Filtered<Base, Clamp>::add_weighted(int2(x, y), wx2 * wy2, clamped, isolated, bounds);
    Filtered<Base, Clamp>::add_weighted(int2(x + 1, y), wx3 * wy2, clamped, isolated, bounds);
    Filtered<Base, Clamp>::add_weighted(int2(x + 2, y), wx4 * wy2, clamped, isolated, bounds);

    // 4. row
    Filtered<Base, Clamp>::add_weighted(int2(x - 2, y + 1), wx0 * wy3, clamped, isolated, bounds);
    Filtered<Base, Clamp>::add_weighted(int2(x - 1, y + 1), wx1 * wy3, clamped, isolated, bounds);
    Filtered<Base, Clamp>::add_weighted(int2(x, y + 1), wx2 * wy3, clamped, isolated, bounds);
    Filtered<Base, Clamp>::add_weighted(int2(x + 1, y + 1), wx3 * wy3, clamped, isolated, bounds);
    Filtered<Base, Clamp>::add_weighted(int2(x + 2, y + 1), wx4 * wy3, clamped, isolated, bounds);

    // 5. row
    Filtered<Base, Clamp>::add_weighted(int2(x - 2, y + 2), wx0 * wy4, clamped, isolated, bounds);
    Filtered<Base, Clamp>::add_weighted(int2(x - 1, y + 2), wx1 * wy4, clamped, isolated, bounds);
    Filtered<Base, Clamp>::add_weighted(int2(x, y + 2), wx2 * wy4, clamped, isolated, bounds);
    Filtered<Base, Clamp>::add_weighted(int2(x + 1, y + 2), wx3 * wy4, clamped, isolated, bounds);
    Filtered<Base, Clamp>::add_weighted(int2(x + 2, y + 2), wx4 * wy4, clamped, isolated, bounds);
}

template <class Base, class Clamp>
Filtered_inf<Base, Clamp>::Filtered_inf(int2 dimensions, float exposure, const Clamp& clamp,
                                        filter::Filter const* filter) noexcept
    : Filtered<Base, Clamp>(dimensions, exposure, clamp, filter) {}

template <class Base, class Clamp>
Filtered_inf<Base, Clamp>::Filtered_inf(int2 dimensions, float exposure,
                                        Texture_ptr const& backplate, const Clamp& clamp,
                                        filter::Filter const* filter) noexcept
    : Filtered<Base, Clamp>(dimensions, exposure, backplate, clamp, filter) {}

template <class Base, class Clamp>
void Filtered_inf<Base, Clamp>::add_sample(sampler::Camera_sample const& sample,
                                           float4 const& color, int4 const& isolated,
                                           int4 const& bounds) noexcept {
    float4 const clamped = Filtered<Base, Clamp>::clamp_.clamp(color);

    int32_t const px = bounds[0] + sample.pixel[0];
    int32_t const py = bounds[1] + sample.pixel[1];

    int32_t const r  = Filtered<Base, Clamp>::filter_radius_int();
    float const   rf = Filtered<Base, Clamp>::filter_->radius();

    for (int32_t ky = -r; ky <= r; ++ky) {
        for (int32_t kx = -r; kx <= r; ++kx) {
            int2 const pixel(px + kx, py + ky);

            float2 const offset = sample.pixel_uv - 0.5f - float2(kx, ky);

            if (offset[0] < rf && offset[1] < rf) {
                Filtered<Base, Clamp>::weight_and_add(pixel, offset, clamped, isolated, bounds);
            }
        }
    }
}

}  // namespace rendering::sensor

#endif
