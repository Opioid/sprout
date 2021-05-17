#include "distribution_1d.hpp"

#include <algorithm>
#include <cmath>

#include "debug/assert.hpp"

namespace math {

// The initial motivation for this version comes from the following article:
// https://dirtyhandscoding.wordpress.com/2017/08/25/performance-comparison-linear-search-vs-binary-search/

Distribution_1D::Distribution_1D()
    : size_(0), lut_size_(0), integral_(-1.f), lut_range_(0.f), cdf_(nullptr), lut_(nullptr) {}

Distribution_1D::Distribution_1D(Distribution_1D&& other)
    : size_(other.size_),
      lut_size_(other.lut_size_),
      integral_(other.integral_),
      lut_range_(other.lut_range_),
      cdf_(other.cdf_),
      lut_(other.lut_) {
    other.cdf_ = nullptr;
    other.lut_ = nullptr;
}

Distribution_1D::~Distribution_1D() {
    delete[] lut_;
    delete[] cdf_;
}

void Distribution_1D::init(float const* data, uint32_t len, uint32_t lut_bucket_size) {
    precompute_1D_pdf_cdf(data, len);

    uint32_t lut_size = 0 == lut_bucket_size ? len / 16 : len / lut_bucket_size;

    lut_size = std::min(std::max(lut_size, 1u), size_ + 1);

    init_lut(lut_size);
}

void Distribution_1D::precompute_1D_pdf_cdf(float const* data, uint32_t len) {
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

void Distribution_1D::init_lut(uint32_t lut_size) {
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

}  // namespace math
