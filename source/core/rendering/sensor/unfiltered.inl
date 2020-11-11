#ifndef SU_CORE_RENDERING_SENSOR_UNFILTERED_INL
#define SU_CORE_RENDERING_SENSOR_UNFILTERED_INL

#include "aov/value.inl"
#include "base/math/vector4.inl"
#include "sampler/camera_sample.hpp"
#include "unfiltered.hpp"

namespace rendering::sensor {

template <class Base, class Clamp>
Unfiltered<Base, Clamp>::Unfiltered(Clamp const& clamp, bool adaptive)
    : Base(0, adaptive), clamp_(clamp) {}

template <class Base, class Clamp>
float4 Unfiltered<Base, Clamp>::add_sample(Sample const& sample, float4 const& color,
                                           aov::Value const* aov, int4 const& /*isolated*/,
                                           int2              offset, int4 const& /*bounds*/) {
    int2 const pixel = offset + sample.pixel;

    float4 const clamped = clamp_.clamp(color);

    Base::add_pixel(pixel, clamped, 1.f);

    if (aov) {
        for (uint32_t i = 0, len = aov->num_slots(); i < len; ++i) {
            auto const r = aov->value(i);

            Base::add_AOV(pixel, i, r, 1.f);
        }
    }

    return clamped;
}

template <class Base, class Clamp>
void Unfiltered<Base, Clamp>::splat_sample(Sample_to const& sample, float4 const& color,
                                           int2 offset, int4 const& /*bounds*/) {
    Base::splat_pixel_atomic(offset + sample.pixel, clamp_.clamp(color), 1.f);
}

}  // namespace rendering::sensor

#endif
