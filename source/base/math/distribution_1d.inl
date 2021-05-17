#ifndef SU_BASE_MATH_DISTRIBUTION_DISTRIBUTION_1D_INL
#define SU_BASE_MATH_DISTRIBUTION_DISTRIBUTION_1D_INL

#include "distribution_1d.hpp"

#include <algorithm>
#include <cmath>

#include "debug/assert.hpp"

namespace math {

// The initial motivation for this version comes from the following article:
// https://dirtyhandscoding.wordpress.com/2017/08/25/performance-comparison-linear-search-vs-binary-search/

inline float Distribution_1D::integral() const {
    return integral_;
}

static inline uint32_t search(float const* buffer, uint32_t begin, uint32_t end, float key) {
    for (uint32_t i = begin; i < end; ++i) {
        if (buffer[i] >= key) {
            return i;
        }
    }

    return end;
}

inline uint32_t Distribution_1D::sample(float r) const {
    uint32_t const bucket = map(r);
    uint32_t const begin  = lut_[bucket];
    uint32_t const it     = search(cdf_, begin, size_, r);

    return (0 == it) ? 0 : it - 1;
}

inline Distribution_1D::Discrete Distribution_1D::sample_discrete(float r) const {
    uint32_t const offset = sample(r);

    SOFT_ASSERT(offset < size_ + 1);

    return {offset, cdf_[offset + 1] - cdf_[offset]};
}

inline Distribution_1D::Continuous Distribution_1D::sample_continuous(float r) const {
    uint32_t const offset = sample(r);

    SOFT_ASSERT(offset < size_ + 1);

    float const c = cdf_[offset + 1];
    float const v = c - cdf_[offset];

    if (0.f == v) {
        return {0.f, 0.f};
    }

    float const t = (c - r) / v;

    float const result = (float(offset) + t) / float(size_);

    return {result, v};
}

inline float Distribution_1D::pdf(uint32_t index) const {
    SOFT_ASSERT(index < size_ + 1);

    return cdf_[index + 1] - cdf_[index];
}

inline float Distribution_1D::pdf(float u) const {
    uint32_t const offset = uint32_t(u * float(size_));

    SOFT_ASSERT(offset < size_ + 1);

    return cdf_[offset + 1] - cdf_[offset];
}

inline uint32_t Distribution_1D::lut_size() const {
    return lut_size_ - 2;
}

inline uint32_t Distribution_1D::map(float s) const {
    return uint32_t(s * lut_range_);
}

template <uint32_t N>
Distribution_1D::Discrete distribution_sample_discrete(float data[N], uint32_t n, float r) {
    float integral = 0.f;
    for (uint32_t i = 0; i < N; ++i) {
        integral += data[i];
    }

    float const ii = 1.f / integral;

    float cdf[N + 1];

    cdf[0] = 0.f;
    for (uint32_t i = 1; i < N; ++i) {
        cdf[i] = std::fma(data[i - 1], ii, cdf[i - 1]);
    }
    cdf[N] = 1.f;

    uint32_t const it     = search(cdf, 0, n, r);
    uint32_t const offset = (0 != it) ? it - 1 : 0;

    SOFT_ASSERT(offset < N);

    return {offset, cdf[offset + 1] - cdf[offset]};
}

template <uint32_t N>
float distribution_pdf(float data[N], uint32_t index) {
    float integral = 0.f;
    for (uint32_t i = 0; i < N; ++i) {
        integral += data[i];
    }

    float const ii = 1.f / integral;

    float cdf[N + 1];

    cdf[0] = 0.f;
    for (uint32_t i = 1; i < N; ++i) {
        cdf[i] = std::fma(data[i - 1], ii, cdf[i - 1]);
    }
    cdf[N] = 1.f;

    SOFT_ASSERT(index < N);

    return cdf[index + 1] - cdf[index];
}

}  // namespace math

#endif
