#ifndef SU_BASE_MATH_DISTRIBUTION_DISTRIBUTION_1D_INL
#define SU_BASE_MATH_DISTRIBUTION_DISTRIBUTION_1D_INL

#include "distribution_1d.hpp"

#include <algorithm>
#include <cmath>

#include "debug/assert.hpp"

namespace math {

// The initial motivation for this version comes from the following article:
// https://dirtyhandscoding.wordpress.com/2017/08/25/performance-comparison-linear-search-vs-binary-search/

inline Distribution_1D::Distribution_1D()
    : size_(0), lut_size_(0), integral_(-1.f), lut_range_(0.f), cdf_(nullptr), lut_(nullptr) {}

inline Distribution_1D::~Distribution_1D() {
    delete[] lut_;
    delete[] cdf_;
}

inline void Distribution_1D::init(float const* data, uint32_t len, uint32_t lut_bucket_size) {
    precompute_1D_pdf_cdf(data, len);

    uint32_t lut_size = 0 == lut_bucket_size ? len / 16 : len / lut_bucket_size;

    lut_size = std::min(std::max(lut_size, 1u), size_ + 1);

    init_lut(lut_size);
}

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

inline void Distribution_1D::precompute_1D_pdf_cdf(float const* data, uint32_t len) {
    float integral = 0.f;
    for (uint32_t i = 0; i < len; ++i) {
        integral += data[i];
    }

    if (0.f == integral) {
        if (0.f != integral_) {
            delete[] cdf_;

            uint32_t const size = 1;

            size_ = size;
            cdf_  = new float[size + 1];

            cdf_[0] = 1.f;
            cdf_[1] = 1.f;

            integral_ = 0.f;
        }

        return;
    }

    if (size_ != len) {
        delete[] cdf_;

        size_ = len;
        cdf_  = new float[len + 1];
    }

    float const ii = 1.f / integral;

    cdf_[0] = 0.f;
    for (uint32_t i = 1; i < len; ++i) {
        cdf_[i] = std::fma(data[i - 1], ii, cdf_[i - 1]);
    }
    cdf_[len] = 1.f;

    integral_ = integral;
}

inline void Distribution_1D::init_lut(uint32_t lut_size) {
    uint32_t const padded_lut_size = lut_size + 2;

    if (padded_lut_size != lut_size_) {
        delete[] lut_;

        lut_size_  = padded_lut_size;
        lut_       = new uint32_t[padded_lut_size];
        lut_range_ = float(lut_size);
    }

    lut_[0] = 0;

    uint32_t border = 0;
    uint32_t last   = 0;

    for (uint32_t i = 1, len = size_ + 1; i < len; ++i) {
        if (uint32_t const mapped = map(cdf_[i]); mapped > border) {
            last = i;

            for (uint32_t j = border + 1; j <= mapped; ++j) {
                lut_[j] = last;
            }

            border = mapped;
        }
    }

    for (uint32_t i = border + 1; i < padded_lut_size; ++i) {
        lut_[i] = last;
    }
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
