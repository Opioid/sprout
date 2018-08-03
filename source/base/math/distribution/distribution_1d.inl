#ifndef SU_BASE_MATH_DISTRIBUTION_DISTRIBUTION_1D_INL
#define SU_BASE_MATH_DISTRIBUTION_DISTRIBUTION_1D_INL

#include <algorithm>
#include "distribution_1d.hpp"
#include "memory/align.hpp"

#include "debug/assert.hpp"

namespace math {

inline void Distribution_1D::init(float const* data, size_t len) noexcept {
    precompute_1D_pdf_cdf(data, len);
}

inline float Distribution_1D::integral() const noexcept {
    return integral_;
}

inline uint32_t Distribution_1D::sample(float r) const noexcept {
    auto it = std::lower_bound(cdf_.begin(), cdf_.end(), r);

    uint32_t offset = 0;
    if (it != cdf_.begin()) {
        offset = static_cast<uint32_t>(it - cdf_.begin() - 1);
    }

    return offset;
}

inline Distribution_1D::Discrete Distribution_1D::sample_discrete(float r) const noexcept {
    uint32_t const offset = sample(r);

    return {offset, pdf_[offset]};
}

inline Distribution_1D::Continuous Distribution_1D::sample_continuous(float r) const noexcept {
    uint32_t const offset = sample(r);

    float const c = cdf_[offset + 1];
    float const t = (c - r) / (c - cdf_[offset]);

    return {(static_cast<float>(offset) + t) / size_, pdf_[offset]};
}

inline float Distribution_1D::pdf(uint32_t index) const noexcept {
    return pdf_[index];
}

inline float Distribution_1D::pdf(float u) const noexcept {
    uint32_t const offset = static_cast<uint32_t>(u * size_);

    return pdf_[offset];
}

inline size_t Distribution_1D::num_bytes() const noexcept {
    return sizeof(*this) + sizeof(float) * (pdf_.size() + cdf_.size());
}

inline void Distribution_1D::precompute_1D_pdf_cdf(float const* data, size_t len) noexcept {
    float integral = 0.f;
    for (size_t i = 0; i < len; ++i) {
        integral += data[i];
    }

    if (0.f == integral) {
        pdf_.resize(1, 0.f);

        cdf_.resize(3);
        cdf_[0] = 0.f;
        cdf_[1] = 1.f;
        cdf_[2] = 1.f;

        integral_ = 0.f;
        size_     = 1.f;

        return;
    }

    pdf_.resize(len);
    cdf_.resize(len + 2);

    for (size_t i = 0; i < len; ++i) {
        pdf_[i] = data[i] / integral;
    }

    cdf_[0] = 0.f;
    for (size_t i = 1; i < len; ++i) {
        cdf_[i] = cdf_[i - 1] + pdf_[i - 1];
    }
    cdf_[len] = 1.f;
    // This takes care of corner case: pdf(1)
    cdf_[len + 1] = 1.f;

    integral_ = integral;

    size_ = static_cast<float>(len);
}

//==================================================================================================

inline void Distribution_lut_1D::init(float const* data, uint32_t len, uint32_t lut_bucket_size) noexcept {
    precompute_1D_pdf_cdf(data, len);

    uint32_t lut_size = 0 == lut_bucket_size ? len / 16 : len / lut_bucket_size;

    lut_size = std::min(std::max(lut_size, 1u), static_cast<uint32_t>(pdf_.size()));

    init_lut(lut_size);
}

inline float Distribution_lut_1D::integral() const noexcept {
    return integral_;
}

inline uint32_t Distribution_lut_1D::sample(float r) const noexcept {
    uint32_t const bucket = map(r);

    uint32_t const begin = lut_[bucket];
    uint32_t const end   = lut_[bucket + 1];

    auto const it = std::lower_bound(cdf_.begin() + begin, cdf_.begin() + end, r);

    if (it != cdf_.begin()) {
        return static_cast<uint32_t>(it - cdf_.begin() - 1);
    }

    return 0;
}

inline Distribution_lut_1D::Discrete Distribution_lut_1D::sample_discrete(float r) const noexcept {
    uint32_t const offset = sample(r);

    return {offset, pdf_[offset]};
}

inline Distribution_lut_1D::Continuous Distribution_lut_1D::sample_continuous(float r) const noexcept {
    uint32_t const offset = sample(r);

    float const c = cdf_[offset + 1];
    float const t = (c - r) / (c - cdf_[offset]);

    return {(static_cast<float>(offset) + t) / size_, pdf_[offset]};
}

inline float Distribution_lut_1D::pdf(uint32_t index) const noexcept {
    return pdf_[index];
}

inline float Distribution_lut_1D::pdf(float u) const noexcept {
    uint32_t const offset = static_cast<uint32_t>(u * size_);

    return pdf_[offset];
}

inline size_t Distribution_lut_1D::num_bytes() const noexcept {
    return sizeof(*this) + sizeof(float) * (pdf_.size() + cdf_.size()) +
           sizeof(uint32_t) * lut_.size();
}

inline uint32_t Distribution_lut_1D::map(float s) const noexcept {
    return static_cast<uint32_t>(s * lut_range_);
}

inline void Distribution_lut_1D::precompute_1D_pdf_cdf(float const* data, uint32_t len) noexcept {
    float integral = 0.f;
    for (uint32_t i = 0; i < len; ++i) {
        integral += data[i];
    }

    if (0.f == integral) {
        pdf_.resize(1, 0.f);

        cdf_.resize(3);
        cdf_[0] = 0.f;
        cdf_[1] = 1.f;
        cdf_[2] = 1.f;

        integral_ = 0.f;
        size_     = 1.f;

        return;
    }

    pdf_.resize(len);
    cdf_.resize(len + 2);

    for (uint32_t i = 0; i < len; ++i) {
        pdf_[i] = data[i] / integral;
    }

    cdf_[0] = 0.f;
    for (uint32_t i = 1; i < len; ++i) {
        cdf_[i] = cdf_[i - 1] + pdf_[i - 1];
    }
    cdf_[len] = 1.f;
    // This takes care of corner case: pdf(1)
    cdf_[len + 1] = 1.f;

    integral_ = integral;

    size_ = static_cast<float>(len);
}

inline void Distribution_lut_1D::init_lut(uint32_t lut_size) noexcept {
    lut_.resize(lut_size + 2);
    lut_range_ = static_cast<float>(lut_size);

    lut_[0] = 0;

    uint32_t border = 0;
    uint32_t last   = 0;

    for (uint32_t i = 1, len = static_cast<uint32_t>(cdf_.size()); i < len; ++i) {
        uint32_t mapped = map(cdf_[i]);

        if (mapped > border) {
            last = i;

            for (uint32_t j = border + 1; j <= mapped; ++j) {
                lut_[j] = last;
            }

            border = mapped;
        }
    }

    for (uint32_t i = border + 1, len = static_cast<uint32_t>(lut_.size()); i < len; ++i) {
        lut_[i] = last;
    }
}

//==================================================================================================

inline Distribution_implicit_pdf_lut_1D::Distribution_implicit_pdf_lut_1D() noexcept
    : lut_(nullptr), lut_size_(0), cdf_(nullptr), cdf_size_(0) {}

inline Distribution_implicit_pdf_lut_1D::~Distribution_implicit_pdf_lut_1D() noexcept {
    memory::free_aligned(cdf_);
    memory::free_aligned(lut_);
}

inline void Distribution_implicit_pdf_lut_1D::init(float const* data, uint32_t len,
                                                   uint32_t lut_bucket_size) noexcept {
    precompute_1D_pdf_cdf(data, len);

    uint32_t lut_size = 0 == lut_bucket_size ? len / 16 : len / lut_bucket_size;

    lut_size = std::min(std::max(lut_size, 1u), cdf_size_ - 1);

    init_lut(lut_size);
}

inline float Distribution_implicit_pdf_lut_1D::integral() const noexcept {
    return integral_;
}

inline uint32_t Distribution_implicit_pdf_lut_1D::sample(float r) const noexcept {
    uint32_t const bucket = map(r);

    uint32_t const begin = lut_[bucket];
    uint32_t const end   = lut_[bucket + 1];

    float const* it = std::lower_bound(cdf_ + begin, cdf_ + end, r);

    if (it != cdf_) {
        return static_cast<uint32_t>(it - cdf_ - 1);
    }

    return 0;
}

inline Distribution_implicit_pdf_lut_1D::Discrete Distribution_implicit_pdf_lut_1D::sample_discrete(
    float r) const noexcept {
    uint32_t const offset = sample(r);

    return {offset, cdf_[offset + 1] - cdf_[offset]};
}

inline Distribution_implicit_pdf_lut_1D::Continuous
Distribution_implicit_pdf_lut_1D::sample_continuous(float r) const noexcept {
    uint32_t const offset = sample(r);

    float const c = cdf_[offset + 1];
    float const v = c - cdf_[offset];

    if (0.f == v) {
        return {0.f, 0.f};
    }

    float const t = (c - r) / v;

    float const result = (static_cast<float>(offset) + t) / size_;

    return {result, v};
}

inline float Distribution_implicit_pdf_lut_1D::pdf(uint32_t index) const noexcept {
    return cdf_[index + 1] - cdf_[index];
}

inline float Distribution_implicit_pdf_lut_1D::pdf(float u) const noexcept {
    uint32_t const offset = static_cast<uint32_t>(u * size_);

    return cdf_[offset + 1] - cdf_[offset];
}

inline uint32_t Distribution_implicit_pdf_lut_1D::lut_size() const noexcept {
    return lut_size_ - 2;
}

inline size_t Distribution_implicit_pdf_lut_1D::num_bytes() const noexcept {
    return sizeof(*this) + sizeof(float) * cdf_size_ + sizeof(uint32_t) * lut_size_;
}

inline uint32_t Distribution_implicit_pdf_lut_1D::map(float s) const noexcept {
    return static_cast<uint32_t>(s * lut_range_);
}

inline void Distribution_implicit_pdf_lut_1D::precompute_1D_pdf_cdf(float const* data,
                                                                    uint32_t     len) noexcept {
    float integral = 0.f;
    for (uint32_t i = 0; i < len; ++i) {
        integral += data[i];
    }

    if (0.f == integral) {
        cdf_size_ = 3;
        cdf_      = memory::allocate_aligned<float>(cdf_size_);

        cdf_[0] = 0.f;
        cdf_[1] = 1.f;
        cdf_[2] = 1.f;

        integral_ = 0.f;
        size_     = 1.f;

        return;
    }

    cdf_size_ = len + 1;
    cdf_      = memory::allocate_aligned<float>(cdf_size_);

    cdf_[0] = 0.f;
    for (uint32_t i = 1; i < len; ++i) {
        cdf_[i] = cdf_[i - 1] + data[i - 1] / integral;
    }
    cdf_[len] = 1.f;
    // This takes care of corner case: pdf(1)
    cdf_[len + 1] = 1.f;

    integral_ = integral;

    size_ = static_cast<float>(len);
}

inline void Distribution_implicit_pdf_lut_1D::init_lut(uint32_t lut_size) noexcept {
    lut_size_  = lut_size + 2;
    lut_       = memory::allocate_aligned<uint32_t>(lut_size_);
    lut_range_ = static_cast<float>(lut_size);

    lut_[0] = 0;

    uint32_t border = 0;
    uint32_t last   = 0;

    for (uint32_t i = 1, len = cdf_size_; i < len; ++i) {
        uint32_t const mapped = map(cdf_[i]);

        if (mapped > border) {
            last = i;

            for (uint32_t j = border + 1; j <= mapped; ++j) {
                lut_[j] = last;
            }

            border = mapped;
        }
    }

    for (uint32_t i = border + 1, len = lut_size_; i < len; ++i) {
        lut_[i] = last;
    }
}

//==================================================================================================

// The initial motivation for this version comes from the following article:
// https://dirtyhandscoding.wordpress.com/2017/08/25/performance-comparison-linear-search-vs-binary-search/
// For the data I tested the

inline Distribution_implicit_pdf_lut_lin_1D::Distribution_implicit_pdf_lut_lin_1D() noexcept
    : lut_(nullptr), lut_size_(0), cdf_(nullptr), cdf_size_(0) {}

inline Distribution_implicit_pdf_lut_lin_1D::~Distribution_implicit_pdf_lut_lin_1D() noexcept {
    memory::free_aligned(cdf_);
    memory::free_aligned(lut_);
}

inline void Distribution_implicit_pdf_lut_lin_1D::init(float const* data, uint32_t len,
                                                       uint32_t lut_bucket_size) noexcept {
    precompute_1D_pdf_cdf(data, len);

    uint32_t lut_size = 0 == lut_bucket_size ? len / 16 : len / lut_bucket_size;

    lut_size = std::min(std::max(lut_size, 1u), cdf_size_ - 1);

    init_lut(lut_size);
}

inline float Distribution_implicit_pdf_lut_lin_1D::integral() const noexcept {
    return integral_;
}

static inline uint32_t search(float const* buffer, uint32_t begin, /*uint32_t end,*/ float key) noexcept {
    //	for (uint32_t i = begin; i < end; ++i) {
    //		if (buffer[i] >= key) {
    //			return i;
    //		}
    //	}

    //	return end;

    // The loop will terminate eventually, because buffer[len - 1] == 1.f and key <= 1.f
    for (uint32_t i = begin;; ++i) {
        if (buffer[i] >= key) {
            return i;
        }
    }
}

inline uint32_t Distribution_implicit_pdf_lut_lin_1D::sample(float r) const noexcept {
    uint32_t const bucket = map(r);

    uint32_t const begin = lut_[bucket];
    //	uint32_t const end   = lut_[bucket + 1];

    uint32_t const it = search(cdf_, begin, /*end,*/ r);

    if (0 != it) {
        return it - 1;
    }

    return 0;
}

inline Distribution_implicit_pdf_lut_lin_1D::Discrete
Distribution_implicit_pdf_lut_lin_1D::sample_discrete(float r) const noexcept {
    uint32_t const offset = sample(r);

    SOFT_ASSERT(offset + 1 < cdf_size_);

    return {offset, cdf_[offset + 1] - cdf_[offset]};
}

inline Distribution_implicit_pdf_lut_lin_1D::Continuous
Distribution_implicit_pdf_lut_lin_1D::sample_continuous(float r) const noexcept {
    uint32_t const offset = sample(r);

    SOFT_ASSERT(offset + 1 < cdf_size_);

    float const c = cdf_[offset + 1];
    float const v = c - cdf_[offset];

    if (0.f == v) {
        return {0.f, 0.f};
    }

    float const t = (c - r) / v;

    float const result = (static_cast<float>(offset) + t) / size_;

    return {result, v};
}

inline float Distribution_implicit_pdf_lut_lin_1D::pdf(uint32_t index) const noexcept {
    SOFT_ASSERT(index + 1 < cdf_size_);

    return cdf_[index + 1] - cdf_[index];
}

inline float Distribution_implicit_pdf_lut_lin_1D::pdf(float u) const noexcept {
    uint32_t const offset = static_cast<uint32_t>(u * size_);

    SOFT_ASSERT(offset + 1 < cdf_size_);

    return cdf_[offset + 1] - cdf_[offset];
}

inline uint32_t Distribution_implicit_pdf_lut_lin_1D::lut_size() const noexcept {
    return lut_size_ - 2;
}

inline size_t Distribution_implicit_pdf_lut_lin_1D::num_bytes() const noexcept {
    return sizeof(*this) + sizeof(float) * cdf_size_ + sizeof(uint32_t) * lut_size_;
}

inline uint32_t Distribution_implicit_pdf_lut_lin_1D::map(float s) const noexcept {
    return static_cast<uint32_t>(s * lut_range_);
}

inline void Distribution_implicit_pdf_lut_lin_1D::precompute_1D_pdf_cdf(float const* data,
                                                                        uint32_t     len) noexcept {
    float integral = 0.f;
    for (uint32_t i = 0; i < len; ++i) {
        integral += data[i];
    }

    if (0.f == integral) {
        cdf_size_ = 3;
        cdf_      = memory::allocate_aligned<float>(cdf_size_);

        cdf_[0] = 0.f;
        cdf_[1] = 1.f;
        cdf_[2] = 1.f;

        integral_ = 0.f;
        size_     = 1.f;

        return;
    }

    cdf_size_ = len + 2;
    cdf_      = memory::allocate_aligned<float>(cdf_size_);

    cdf_[0] = 0.f;
    for (uint32_t i = 1; i < len; ++i) {
        cdf_[i] = cdf_[i - 1] + data[i - 1] / integral;
    }
    cdf_[len] = 1.f;
    // This takes care of corner case: pdf(1)
    cdf_[len + 1] = 1.f;

    integral_ = integral;

    size_ = static_cast<float>(len);
}

inline void Distribution_implicit_pdf_lut_lin_1D::init_lut(uint32_t lut_size) noexcept {
    lut_size_  = lut_size + 2;
    lut_       = memory::allocate_aligned<uint32_t>(lut_size_);
    lut_range_ = static_cast<float>(lut_size);

    lut_[0] = 0;

    uint32_t border = 0;
    uint32_t last   = 0;

    for (uint32_t i = 1, len = cdf_size_; i < len; ++i) {
        uint32_t const mapped = map(cdf_[i]);

        if (mapped > border) {
            last = i;

            for (uint32_t j = border + 1; j <= mapped; ++j) {
                lut_[j] = last;
            }

            border = mapped;
        }
    }

    for (uint32_t i = border + 1, len = lut_size_; i < len; ++i) {
        lut_[i] = last;
    }
}

}  // namespace math

#endif
