#ifndef SU_BASE_MATH_HALF_INL
#define SU_BASE_MATH_HALF_INL

#include "half.hpp"
#include "simd/simd.hpp"
#include "vector3.inl"

#include <cstring>

namespace math {

static inline int16_t float_to_half(float f) {
    return int16_t(_mm_cvtsi128_si32(_mm_cvtps_ph(_mm_set_ss(f), _MM_FROUND_TO_NEAREST_INT)));
}

static inline int16_t rounded(int32_t value, int32_t g, int32_t s) {
    return int16_t(value + (g & (s | value)));
}

static inline int16_t float_to_half2(float f) {
    int32_t fbits;
    std::memcpy(&fbits, &f, sizeof(float));

    int32_t sign = (fbits >> 16) & 0x8000;
    fbits &= 0x7FFFFFFF;

    if (fbits >= 0x7F800000) {
        return int16_t(sign | 0x7C00 |
                       ((fbits > 0x7F800000) ? (0x200 | ((fbits >> 13) & 0x3FF)) : 0));
    }

    if (fbits >= 0x47800000) {
        return int16_t(sign | 0x7C00);
    }

    if (fbits >= 0x38800000) {
        return rounded(sign | (((fbits >> 23) - 112) << 10) | ((fbits >> 13) & 0x3FF),
                       (fbits >> 12) & 1, (fbits & 0xFFF) != 0);
    }

    if (fbits >= 0x33000000) {
        int i = 125 - (fbits >> 23);
        fbits = (fbits & 0x7FFFFF) | 0x800000;
        return rounded(sign | (fbits >> (i + 1)), (fbits >> i) & 1,
                       (fbits & ((static_cast<int32_t>(1) << i) - 1)) != 0);
    }

    return int16_t(sign);
}

static inline short3 float_to_half(packed_float3 const& f) {
    return short3(float_to_half(f[0]), float_to_half(f[1]), float_to_half(f[2]));
}

static inline float half_to_float(int16_t h) {
    return _mm_cvtss_f32(_mm_cvtph_ps(_mm_cvtsi32_si128(h)));
}

static inline float half_to_float2(int16_t h) {
    int32_t fbits = int32_t(h & 0x8000) << 16;
    int32_t abs   = h & 0x7FFF;

    if (abs) {
        fbits |= 0x38000000 << static_cast<unsigned>(abs >= 0x7C00);

        for (; abs < 0x400; abs <<= 1, fbits -= 0x800000)
            ;

        fbits += int32_t(abs) << 13;
    }

    float out;
    std::memcpy(&out, &fbits, sizeof(float));
    return out;
}

static inline float2 half_to_float(short2 h) {
    return float2(half_to_float(h[0]), half_to_float(h[1]));
}

// static inline float3 half_to_float(short3 h)  {
//    return float3(half_to_float(h[0]), half_to_float(h[1]), half_to_float(h[2]));
//}

static inline float3 half_to_float(short3 h) {
    alignas(16) int32_t const i[4] = {int32_t(h[0]) | int32_t(h[1]) << 16, int32_t(h[2]), 0, 0};

    __m128 const r = _mm_cvtph_ps(_mm_load_si128(reinterpret_cast<__m128i const*>(i)));

    float3 result;
    simd::store_float4(result.v, r);
    return result;
}

// inline half::half(float s)
//    : h(int16_t(_mm_cvtsi128_si32(
//          _mm_cvtps_ph(_mm_set_ss(s), _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC)))) {}

}  // namespace math

#endif
