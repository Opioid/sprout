#ifndef SU_BASE_SIMD_SIMD_INL
#define SU_BASE_SIMD_SIMD_INL

#include "math/vector3.hpp"
#include "simd.hpp"
#include "simd_const.hpp"

namespace simd {

//==============================================================================
// Load operations
//==============================================================================

static inline __m128 set_float4(float x) noexcept {
    return _mm_set1_ps(x);
}

static inline __m128 load_float(float const* x) noexcept {
    return _mm_load_ss(x);
}

static inline __m128 load_float3(float const* source) noexcept {
    // Reads an extra float which is zero'd
    __m128 v = _mm_load_ps(source);
    return _mm_and_ps(v, simd::Mask3);
}

static inline __m128 load_float4(float const* source) noexcept {
    return _mm_load_ps(source);
}

static inline __m128 load_unaligned_float4(float const* source) noexcept {
    return _mm_loadu_ps(source);
}

//==============================================================================
// Store operations
//==============================================================================

static inline float get_x(__m128 v) noexcept {
    return _mm_cvtss_f32(v);
}

static inline void store_float4(float* destination, __m128 v) noexcept {
    _mm_store_ps(destination, v);
}

}  // namespace simd

#endif
