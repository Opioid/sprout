#ifndef SU_CORE_IMAGE_TEXTURE_ADRESS_MODE_HPP
#define SU_CORE_IMAGE_TEXTURE_ADRESS_MODE_HPP

#include "base/math/math.hpp"

namespace image::texture {

struct Address_mode_clamp {
    static float constexpr f(float x) {
        return math::saturate(x);
    }

    static int32_t constexpr increment(int32_t v, int32_t max) {
        if (v >= max) {
            return max;
        }

        return ++v;
    }

    static int32_t constexpr lower_bound(int32_t v, int32_t /*max*/) {
        if (v < 0) {
            return 0;
        }

        return v;
    }
};

struct Address_mode_repeat {
    static float constexpr f(float x) {
        return math::frac(x);
    }

    static int32_t constexpr increment(int32_t v, int32_t max) {
        if (v >= max) {
            return 0;
        }

        return ++v;
    }

    static int32_t constexpr lower_bound(int32_t v, int32_t max) {
        if (v < 0) {
            return max;
        }

        return v;
    }
};

}  // namespace image::texture

#endif
