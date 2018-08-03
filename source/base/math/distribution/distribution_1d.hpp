#ifndef SU_BASE_MATH_DISTRIBUTION_DISTRIBUTION_1D_HPP
#define SU_BASE_MATH_DISTRIBUTION_DISTRIBUTION_1D_HPP

#include <cstddef>
#include <cstdint>
#include <vector>

namespace math {

class Distribution_1D {
  public:
    void init(float const* data, size_t len) noexcept;

    float integral() const noexcept;

    uint32_t sample(float r) const noexcept;

    struct Discrete {
        uint32_t offset;
        float    pdf;
    };
    Discrete sample_discrete(float r) const noexcept;

    struct Continuous {
        float offset;
        float pdf;
    };
    Continuous sample_continuous(float r) const noexcept;

    float pdf(uint32_t index) const noexcept;
    float pdf(float u) const noexcept;

    size_t num_bytes() const noexcept;

  private:
    void precompute_1D_pdf_cdf(float const* data, size_t len) noexcept;

    std::vector<float> pdf_;
    std::vector<float> cdf_;

    float integral_;
    float size_;
};

// LUT implementation inspired by
// https://geidav.wordpress.com/2013/12/29/optimizing-binary-search/

class Distribution_lut_1D {
  public:
    void init(float const* data, uint32_t len, uint32_t lut_bucket_size = 0) noexcept;

    float integral() const noexcept;

    uint32_t sample(float r) const noexcept;

    struct Discrete {
        uint32_t offset;
        float    pdf;
    };
    Discrete sample_discrete(float r) const noexcept;

    struct Continuous {
        float offset;
        float pdf;
    };
    Continuous sample_continuous(float r) const noexcept;

    float pdf(uint32_t index) const noexcept;
    float pdf(float u) const noexcept;

    size_t num_bytes() const noexcept;

  private:
    uint32_t map(float s) const noexcept;

    void precompute_1D_pdf_cdf(float const* data, uint32_t len) noexcept;

    void init_lut(uint32_t lut_size) noexcept;

    std::vector<uint32_t> lut_;
    std::vector<float>    pdf_;
    std::vector<float>    cdf_;

    float integral_;
    float size_;

    float lut_range_;
};

class Distribution_implicit_pdf_lut_1D {
  public:
    Distribution_implicit_pdf_lut_1D() noexcept;

    ~Distribution_implicit_pdf_lut_1D() noexcept;

    void init(float const* data, uint32_t len, uint32_t lut_bucket_size = 0) noexcept;

    float integral() const noexcept;

    uint32_t sample(float r) const noexcept;

    struct Discrete {
        uint32_t offset;
        float    pdf;
    };
    Discrete sample_discrete(float r) const noexcept;

    struct Continuous {
        float offset;
        float pdf;
    };
    Continuous sample_continuous(float r) const noexcept;

    float pdf(uint32_t index) const noexcept;
    float pdf(float u) const noexcept;

    uint32_t lut_size() const noexcept;

    size_t num_bytes() const noexcept;

  private:
    uint32_t map(float s) const noexcept;

    void precompute_1D_pdf_cdf(float const* data, uint32_t len) noexcept;

    void init_lut(uint32_t lut_size) noexcept;

    uint32_t* lut_;
    uint32_t  lut_size_;

    float*   cdf_;
    uint32_t cdf_size_;

    float integral_;
    float size_;

    float lut_range_;
};

class Distribution_implicit_pdf_lut_lin_1D {
  public:
    Distribution_implicit_pdf_lut_lin_1D() noexcept;

    ~Distribution_implicit_pdf_lut_lin_1D() noexcept;

    void init(float const* data, uint32_t len, uint32_t lut_bucket_size = 0) noexcept;

    float integral() const noexcept;

    uint32_t sample(float r) const noexcept;

    struct Discrete {
        uint32_t offset;
        float    pdf;
    };
    Discrete sample_discrete(float r) const noexcept;

    struct Continuous {
        float offset;
        float pdf;
    };
    Continuous sample_continuous(float r) const noexcept;

    float pdf(uint32_t index) const noexcept;
    float pdf(float u) const noexcept;

    uint32_t lut_size() const noexcept;

    size_t num_bytes() const noexcept;

  private:
    uint32_t map(float s) const noexcept;

    void precompute_1D_pdf_cdf(float const* data, uint32_t len) noexcept;

    void init_lut(uint32_t lut_size) noexcept;

    uint32_t* lut_;
    uint32_t  lut_size_;

    float*   cdf_;
    uint32_t cdf_size_;

    float integral_;
    float size_;

    float lut_range_;
};

}  // namespace math

#endif
