#ifndef SU_BASE_SIMD_SIMD_INL
#define SU_BASE_SIMD_SIMD_INL

#include "math/vector3.hpp"
#include "simd.hpp"
#include "simd_const.hpp"

namespace simd {

//==============================================================================
// Load operations
//==============================================================================

static inline Vector SU_CALLCONV set_float4(float x) noexcept {
    return _mm_set1_ps(x);
}

static inline Vector SU_CALLCONV load_float(float const* x) noexcept {
    return _mm_load_ss(x);
}

static inline Vector SU_CALLCONV load_float3(const math::Vector3<float>& source) noexcept {
    __m128 x  = _mm_load_ss(&source.v[0]);
    __m128 y  = _mm_load_ss(&source.v[1]);
    __m128 z  = _mm_load_ss(&source.v[2]);
    __m128 xy = _mm_unpacklo_ps(x, y);
    return _mm_movelh_ps(xy, z);
}

static inline Vector SU_CALLCONV load_float3(float const* source) noexcept {
    // Reads an extra float which is zero'd
    __m128 v = _mm_load_ps(source);
    return _mm_and_ps(v, simd::Mask3);
}

static inline Vector SU_CALLCONV load_float4(float const* source) noexcept {
    return _mm_load_ps(source);
}

static inline Vector SU_CALLCONV load_unaligned_float4(float const* source) noexcept {
    return _mm_loadu_ps(source);
}

//==============================================================================
// Store operations
//==============================================================================

static inline float SU_CALLCONV get_x(FVector v) noexcept {
    return _mm_cvtss_f32(v);
}

static inline void SU_CALLCONV store_float3(math::Vector3<float>& destination, FVector v) noexcept {
    Vector t1 = SU_PERMUTE_PS(v, _MM_SHUFFLE(1, 1, 1, 1));
    Vector t2 = SU_PERMUTE_PS(v, _MM_SHUFFLE(2, 2, 2, 2));
    _mm_store_ss(&destination.v[0], v);
    _mm_store_ss(&destination.v[1], t1);
    _mm_store_ss(&destination.v[2], t2);
}

// static inline void SU_CALLCONV store_float3(Vector3f_a& destination, FVector v) {
//	Vector t = SU_PERMUTE_PS(v, _MM_SHUFFLE(2, 2, 2, 2));
//	_mm_storel_epi64(reinterpret_cast<__m128i*>(&destination), _mm_castps_si128(v));
//	_mm_store_ss(&destination[2], t);
//}

static inline void SU_CALLCONV store_float4(float* destination, FVector v) noexcept {
    _mm_store_ps(destination, v);
}

}  // namespace simd

#endif
