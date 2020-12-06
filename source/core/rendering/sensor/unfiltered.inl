#ifndef SU_CORE_RENDERING_SENSOR_UNFILTERED_INL
#define SU_CORE_RENDERING_SENSOR_UNFILTERED_INL

#include "aov/value.inl"
#include "base/math/vector4.inl"
#include "sampler/camera_sample.hpp"
#include "unfiltered.hpp"

namespace rendering::sensor {

template <class Base, class Clamp>
Unfiltered<Base, Clamp>::Unfiltered(Clamp const& clamp) : Base(0), clamp_(clamp) {}

template <class Base, class Clamp>
void Unfiltered<Base, Clamp>::add_sample(Sample const& sample, float4_p color,
                                         aov::Value const* aov, int4_p /*isolated*/, int2 offset,
                                         int4_p /*bounds*/) {
    int2 const pixel = offset + sample.pixel;

    Base::add_pixel(pixel, clamp_.clamp(color), 1.f);

    if (aov) {
        for (uint32_t i = 0, len = aov->num_slots(); i < len; ++i) {
            auto const v = aov->value(i);

            aov::Operation const op = aov->operation(i);

            if (aov::Operation::Accumulate == op) {
                Base::add_AOV(pixel, i, v, 1.f);
            } else if (aov::Operation::Overwrite == op) {
                Base::overwrite_AOV(pixel, i, v);
            } else {
                Base::less_AOV(pixel, i, v);
            }
        }
    }
}

template <class Base, class Clamp>
void Unfiltered<Base, Clamp>::splat_sample(Sample_to const& sample, float4_p color, int2 offset,
                                           int4_p /*bounds*/) {
    Base::splat_pixel_atomic(offset + sample.pixel, clamp_.clamp(color), 1.f);
}

}  // namespace rendering::sensor

#endif
