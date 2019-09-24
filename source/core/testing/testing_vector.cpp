#include "testing_vector.hpp"
#include "base/chrono/chrono.hpp"
#include "base/math/vector3.inl"
#include "base/random/generator.inl"
#include "base/string/string.hpp"

#include <iostream>
#include "base/math/print.hpp"

#include <pmmintrin.h>

namespace testing {

struct scalar {
    scalar() noexcept = default;

    scalar(__m128 m) : s(m) {}

    scalar(float i) : s(_mm_set1_ps(i)) {}

    __m128 s;
};

scalar operator*(scalar const& a, scalar const& b) noexcept {
    return _mm_mul_ps(a.s, b.s);
}

scalar operator+(scalar const& a, scalar const& b) noexcept {
    return _mm_add_ps(a.s, b.s);
}

static inline scalar rsqrt(scalar x) noexcept {
    scalar res  = _mm_rsqrt_ps(x.s);
    scalar muls = _mm_mul_ps(_mm_mul_ps(x.s, res.s), res.s);
    return _mm_mul_ps(_mm_mul_ps(simd::Half, res.s), _mm_sub_ps(simd::Three, muls.s));
}

struct vec3 {
    vec3() noexcept = default;

    vec3(__m128 m) noexcept : v(m) {}

    vec3(float s) noexcept : v(_mm_set1_ps(s)) {}

    //    vec3(float sx, float sy, float sz) noexcept {
    //        __m128 x  = _mm_load_ss(&sx);
    //        __m128 y  = _mm_load_ss(&sy);
    //        __m128 z  = _mm_load_ss(&sz);
    //        __m128 xy = _mm_unpacklo_ps(x, y);
    //        v         = _mm_movelh_ps(xy, z);
    //    }

    vec3(float sx, float sy, float sz) noexcept : v(_mm_set_ps(0.f, sz, sy, sx)) {}

    __m128 v;
};

vec3 operator*(scalar const& s, vec3 const& v) noexcept {
    return _mm_mul_ps(s.s, v.v);
}

vec3 operator/(vec3 const& v, scalar const& s) noexcept {
    return _mm_div_ps(v.v, s.s);
}

vec3 operator*(vec3 const& a, vec3 const& b) noexcept {
    return _mm_mul_ps(a.v, b.v);
}

vec3 operator+(vec3 const& a, vec3 const& b) noexcept {
    return _mm_add_ps(a.v, b.v);
}

scalar dot(vec3 const& a, vec3 const& b) noexcept {
    vec3   mul  = _mm_mul_ps(a.v, b.v);
    vec3   shuf = _mm_movehdup_ps(mul.v);
    scalar sums = _mm_add_ss(mul.v, shuf.v);
    shuf        = _mm_movehl_ps(shuf.v, sums.s);
    scalar d    = _mm_add_ss(sums.s, shuf.v);
    // Splat x
    return SU_PERMUTE_PS(d.s, _MM_SHUFFLE(0, 0, 0, 0));
}

vec3 cross(vec3 const& a, vec3 const& b) noexcept {
    vec3 tmp0 = _mm_shuffle_ps(b.v, b.v, _MM_SHUFFLE(3, 0, 2, 1));
    vec3 tmp1 = _mm_shuffle_ps(a.v, a.v, _MM_SHUFFLE(3, 0, 2, 1));

    tmp0 = _mm_mul_ps(tmp0.v, a.v);
    tmp1 = _mm_mul_ps(tmp1.v, b.v);

    vec3 tmp2 = _mm_sub_ps(tmp0.v, tmp1.v);

    return _mm_shuffle_ps(tmp2.v, tmp2.v, _MM_SHUFFLE(3, 0, 2, 1));
}

static inline vec3 normalize(vec3 v) {
    return rsqrt(dot(v, v)) * v;
}

void vector() noexcept {
    std::cout << "vector" << std::endl;

    size_t constexpr Num_iterations = 1024 * 1024 * 10;
    size_t constexpr Num_values     = Num_iterations * (4 + 4);

    float* values = new float[Num_values];

    rnd::Generator rng(0, 0);

    for (size_t i = 0; i < Num_values; ++i) {
        values[i] = rng.random_float();
    }

    {
        auto const start = std::chrono::high_resolution_clock::now();

        float3 accum(0.f);

        for (size_t i = 0; i < Num_iterations; ++i) {
            size_t const o = i * 8;

            float3 const va(values[o + 0], values[o + 1], values[o + 2]);
            float3 const vb(values[o + 3], values[o + 4], values[o + 5]);

            float const sa = values[o + 6];
            float const sb = values[o + 7];

            float3 const vc = sa * va;
            float3 const vd = sb * vb;

            float3 const ve = (vc + vd) / (1.f + sa * sb);

            float3 const vf = cross(ve, va);
            float3 const vg = cross(ve, vb);

            accum = normalize(dot(vf, vg) * (accum + vf + vg));
        }

        auto const duration = chrono::seconds_since(start);
        std::cout << "float3 " << accum << " in " << string::to_string(duration) << " s"
                  << std::endl;
    }

    {
        auto const start = std::chrono::high_resolution_clock::now();

        packed_float3 accum(0.f);

        for (size_t i = 0; i < Num_iterations; ++i) {
            size_t const o = i * 8;

            packed_float3 const va(values[o + 0], values[o + 1], values[o + 2]);
            packed_float3 const vb(values[o + 3], values[o + 4], values[o + 5]);

            float const sa = values[o + 6];
            float const sb = values[o + 7];

            packed_float3 const vc = sa * va;
            packed_float3 const vd = sb * vb;

            packed_float3 const ve = (vc + vd) / (1.f + sa * sb);

            packed_float3 const vf = cross(ve, va);
            packed_float3 const vg = cross(ve, vb);

            accum = normalize(dot(vf, vg) * (accum + vf + vg));
        }

        auto const duration = chrono::seconds_since(start);
        std::cout << "packed_float3" << accum << " in " << string::to_string(duration) << " s"
                  << std::endl;
    }

    {
        auto const start = std::chrono::high_resolution_clock::now();

        vec3 accum(0.f);

        for (size_t i = 0; i < Num_iterations; ++i) {
            size_t const o = i * 8;

            vec3 const va(values[o + 0], values[o + 1], values[o + 2]);
            vec3 const vb(values[o + 3], values[o + 4], values[o + 5]);

            scalar const sa = values[o + 6];
            scalar const sb = values[o + 7];

            vec3 const vc = sa * va;
            vec3 const vd = sb * vb;

            vec3 const ve = (vc + vd) / (1.f + sa * sb);

            vec3 const vf = cross(ve, va);
            vec3 const vg = cross(ve, vb);

            accum = normalize(dot(vf, vg) * (accum + vf + vg));
        }

        auto const duration = chrono::seconds_since(start);

        float3 majong;

        _mm_store_ps(majong.v, accum.v);

        std::cout << "vec3 " << majong << " in " << string::to_string(duration) << " s"
                  << std::endl;
    }

    {
        auto const start = std::chrono::high_resolution_clock::now();

        float3 accum(0.f);

        for (size_t i = 0; i < Num_iterations; ++i) {
            size_t const o = i * 8;

            float3 const va(values[o + 0], values[o + 1], values[o + 2]);
            float3 const vb(values[o + 3], values[o + 4], values[o + 5]);

            float const sa = values[o + 6];
            float const sb = values[o + 7];

            float3 const vc = sa * va;
            float3 const vd = sb * vb;

            float3 const ve = (vc + vd) / (1.f + sa * sb);

            float3 const vf = cross(ve, va);
            float3 const vg = cross(ve, vb);

            accum = normalize(dot(vf, vg) * (accum + vf + vg));
        }

        auto const duration = chrono::seconds_since(start);
        std::cout << "float3 " << accum << " in " << string::to_string(duration) << " s"
                  << std::endl;
    }

    {
        auto const start = std::chrono::high_resolution_clock::now();

        Simd3f accum(0.f);

        for (size_t i = 0; i < Num_iterations; ++i) {
            size_t const o = i * 8;

            Simd3f const va(values[o + 0], values[o + 1], values[o + 2]);
            Simd3f const vb(values[o + 3], values[o + 4], values[o + 5]);

            Simd3f const sa = Simd3f(values[o + 6]);
            Simd3f const sb = Simd3f(values[o + 7]);

            Simd3f const vc = sa * va;
            Simd3f const vd = sb * vb;

            Simd3f const ve = (vc + vd) / (1.f + sa * sb);

            Simd3f const vf = cross(ve, va);
            Simd3f const vg = cross(ve, vb);

            accum = normalize(dot(vf, vg) * (accum + vf + vg));
        }

        auto const duration = chrono::seconds_since(start);

        float3 majong;

        _mm_store_ps(majong.v, accum.v);

        std::cout << "Simd3f " << majong << " in " << string::to_string(duration) << " s"
                  << std::endl;
    }

    delete[] values;
}

}  // namespace testing
