#pragma once

#include "base/math/math.hpp"

namespace image::texture::sampler {

struct Address_mode_identity {
    static float f(float x) {
        return x;
    }

    static int32_t increment(int32_t v, int32_t /*max*/) {
        return ++v;
    }

    static int32_t lower_bound(int32_t v, int32_t /*max*/) {
        return v;
    }
};

struct Address_mode_clamp {
    static float f(float x) {
        return math::saturate(x);
    }

    static int32_t increment(int32_t v, int32_t max) {
        if (v >= max) {
            return max;
        }

        return ++v;
    }

    static int32_t lower_bound(int32_t v, int32_t /*max*/) {
        if (v < 0) {
            return 0;
        }

        return v;
    }
};

struct Address_mode_repeat {
    static float f(float x) {
        return math::frac(x);
    }

    static int32_t increment(int32_t v, int32_t max) {
        if (v >= max) {
            return 0;
        }

        return ++v;
    }

    static int32_t lower_bound(int32_t v, int32_t max) {
        if (v < 0) {
            return max;
        }

        return v;
    }
};

}  // namespace image::texture::sampler
