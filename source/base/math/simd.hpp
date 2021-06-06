#ifndef SU_BASE_MATH_SIMD_HPP
#define SU_BASE_MATH_SIMD_HPP

#include "vector.hpp"

#if defined(_MSC_VER)
#include <intrin.h>
#else
#include <x86intrin.h>
#endif

#if defined(_SU_NO_MOVNT_)
#define XM_STREAM_PS(p, a) _mm_store_ps(p, a)
#else
#define SU_STREAM_PS(p, a) _mm_stream_ps(p, a)
#endif

#define SU_PERMUTE_PS(v, c) _mm_shuffle_ps(v, v, c)

// a,b,c,d -> b,c,d,a
#define SU_ROTATE_LEFT(v) _mm_shuffle_ps(v, v, 0x39)

// low{a,b,c,d}|high{e,f,g,h} = {c,d,g,h}
#define SU_MUX_HIGH(low, high) _mm_movehl_ps(low, high)

namespace math {

struct Simdf {
    Simdf() = default;

    Simdf(__m128 m);

    explicit Simdf(float s);

    explicit Simdf(Scalar_p s);

    Simdf(float sx, float sy);

    explicit Simdf(float const* a);

    explicit Simdf(Vector3f_a_p o);

    explicit Simdf(Vector4f_a_p o);

    static Simdf create_from_3(float const* f);

    static Simdf create_from_3_unaligned(float const* f);

    float x() const;
    float y() const;
    float z() const;
    float w() const;

    Simdf splat_x() const;
    Simdf splat_y() const;
    Simdf splat_z() const;
    Simdf splat_w() const;

    __m128 v;
};

}  // namespace math

#endif
