#ifndef SU_CORE_IMAGE_TEXTURE_SAMPLER_ADRESS_MODE_HPP
#define SU_CORE_IMAGE_TEXTURE_SAMPLER_ADRESS_MODE_HPP

#include "base/math/math.hpp"

namespace image::texture::sampler {

struct Address_mode_identity {
    static float f(float x) noexcept {
        return x;
    }

    static int32_t increment(int32_t v, int32_t /*max*/) noexcept {
        return ++v;
    }

    static int32_t lower_bound(int32_t v, int32_t /*max*/) noexcept {
        return v;
    }
};

struct Address_mode_clamp {
    static float f(float x) noexcept {
        return math::saturate(x);
    }

    static int32_t increment(int32_t v, int32_t max) noexcept {
        if (v >= max) {
            return max;
        }

        return ++v;
    }

    static int32_t lower_bound(int32_t v, int32_t /*max*/) noexcept {
        if (v < 0) {
            return 0;
        }

        return v;
    }
};

struct Address_mode_repeat {
    static float f(float x) noexcept {
        return math::frac(x);
    }

    static int32_t increment(int32_t v, int32_t max) noexcept {
        if (v >= max) {
            return 0;
        }

        return ++v;
    }

    static int32_t lower_bound(int32_t v, int32_t max) noexcept {
        if (v < 0) {
            return max;
        }

        return v;
    }
};

}  // namespace image::texture::sampler

#endif
