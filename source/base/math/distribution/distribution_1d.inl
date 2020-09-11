#ifndef SU_BASE_MATH_DISTRIBUTION_DISTRIBUTION_1D_INL
#define SU_BASE_MATH_DISTRIBUTION_DISTRIBUTION_1D_INL

#include "distribution_1d.hpp"

#include <algorithm>

#include "debug/assert.hpp"

namespace math {

// The initial motivation for this version comes from the following article:
// https://dirtyhandscoding.wordpress.com/2017/08/25/performance-comparison-linear-search-vs-binary-search/

inline Distribution_1D::Distribution_1D()
    : lut_size_(0),
      cdf_size_(0),
      lut_(nullptr),
      cdf_(nullptr),
      integral_(-1.f),
      size_(0.f),
      lut_range_(0.f) {}

inline Distribution_1D::~Distribution_1D() {
    delete[] cdf_;
    delete[] lut_;
}

inline void Distribution_1D::init(float const* data, uint32_t len, uint32_t lut_bucket_size) {
    precompute_1D_pdf_cdf(data, len);

    uint32_t lut_size = 0 == lut_bucket_size ? len / 16 : len / lut_bucket_size;

    lut_size = std::min(std::max(lut_size, 1u), cdf_size_ - 1);

    init_lut(lut_size);
}

inline float Distribution_1D::integral() const {
    return integral_;
}

static inline uint32_t search(float const* buffer, uint32_t begin, float key) {
    // The loop will terminate eventually, because buffer[len - 1] == 1.f and key <= 1.f
    for (uint32_t i = begin;; ++i) {
        if (buffer[i] >= key) {
            return i;
        }
    }
}

inline uint32_t Distribution_1D::sample(float r) const {
    uint32_t const bucket = map(r);

    uint32_t const begin = lut_[bucket];

    if (uint32_t const it = search(cdf_, begin, r); 0 != it) {
        return it - 1;
    }

    return 0;
}

inline Distribution_1D::Discrete Distribution_1D::sample_discrete(float r) const {
    uint32_t const offset = sample(r);

    SOFT_ASSERT(offset + 1 < cdf_size_);

    return {offset, cdf_[offset + 1] - cdf_[offset]};
}

inline Distribution_1D::Continuous Distribution_1D::sample_continuous(float r) const {
    uint32_t const offset = sample(r);

    SOFT_ASSERT(offset + 1 < cdf_size_);

    float const c = cdf_[offset + 1];
    float const v = c - cdf_[offset];

    if (0.f == v) {
        return {0.f, 0.f};
    }

    float const t = (c - r) / v;

    float const result = (float(offset) + t) / size_;

    return {result, v};
}

inline float Distribution_1D::pdf(uint32_t index) const {
    SOFT_ASSERT(index + 1 < cdf_size_);

    return cdf_[index + 1] - cdf_[index];
}

inline float Distribution_1D::pdf(float u) const {
    uint32_t const offset = uint32_t(u * size_);

    SOFT_ASSERT(offset + 1 < cdf_size_);

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

    if (0.f == integral && 0.f != integral_) {
        delete[] cdf_;

        uint32_t const cdf_size = 3;

        cdf_size_ = cdf_size;
        cdf_      = new float[cdf_size];

        cdf_[0] = 1.f;
        cdf_[1] = 1.f;
        cdf_[2] = 1.f;

        integral_ = 0.f;
        size_     = 1.f;

        return;
    }

    if (uint32_t const cdf_size = len + 2; cdf_size_ != cdf_size) {
        delete[] cdf_;

        cdf_size_ = cdf_size;
        cdf_      = new float[cdf_size];
    }

    cdf_[0] = 0.f;
    for (uint32_t i = 1; i < len; ++i) {
        cdf_[i] = cdf_[i - 1] + data[i - 1] / integral;
    }
    cdf_[len] = 1.f;
    // This takes care of corner case: pdf(1)
    cdf_[len + 1] = 1.f;

    integral_ = integral;

    size_ = float(len);
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

    for (uint32_t i = 1, len = cdf_size_; i < len; ++i) {
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

static inline uint32_t mini_search(float const* buffer, uint32_t end, float key) {
    uint32_t i = 0;
    for (; i < end; ++i) {
        if (buffer[i] >= key) {
            return i;
        }
    }

    return i;
}

template<uint32_t N>
Distribution_1D::Discrete distribution_sample_discrete(float data[N], uint32_t n, float r) {
    float integral = 0.f;
    for (uint32_t i = 0; i < N; ++i) {
        integral += data[i];
    }

    float cdf[N + 1];

    cdf[0] = 0.f;
    for (uint32_t i = 1; i < N; ++i) {
        cdf[i] = cdf[i - 1] + data[i - 1] / integral;
    }
    cdf[N] = 1.f;

    uint32_t const it = mini_search(cdf, n, r);

    uint32_t const offset = (0 != it) ? it - 1 : 0;

    SOFT_ASSERT(offset < N);

    return {offset, cdf[offset + 1] - cdf[offset]};
}

template<uint32_t N>
float distribution_pdf(float data[N], uint32_t index) {
    float integral = 0.f;
    for (uint32_t i = 0; i < N; ++i) {
        integral += data[i];
    }

    float cdf[N + 1];

    cdf[0] = 0.f;
    for (uint32_t i = 1; i < N; ++i) {
        cdf[i] = cdf[i - 1] + data[i - 1] / integral;
    }
    cdf[N] = 1.f;

    SOFT_ASSERT(index < N);

    return cdf[index + 1] - cdf[index];
}

}  // namespace math

#endif
