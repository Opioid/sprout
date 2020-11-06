#ifndef SU_BASE_MATH_DISTRIBUTION_DISTRIBUTION_1D_HPP
#define SU_BASE_MATH_DISTRIBUTION_DISTRIBUTION_1D_HPP

#include <cstddef>
#include <cstdint>

namespace math {

class Distribution_1D {
  public:
    Distribution_1D();

    ~Distribution_1D();

    void init(float const* data, uint32_t len, uint32_t lut_bucket_size = 0);

    float integral() const;

    uint32_t sample(float r) const;

    struct Discrete {
        uint32_t offset;
        float    pdf;
    };
    Discrete sample_discrete(float r) const;

    struct Continuous {
        float offset;
        float pdf;
    };
    Continuous sample_continuous(float r) const;

    float pdf(uint32_t index) const;
    float pdf(float u) const;

    uint32_t lut_size() const;

  private:
    uint32_t map(float s) const;

    void precompute_1D_pdf_cdf(float const* data, uint32_t len);

    void init_lut(uint32_t lut_size);

    uint32_t lut_size_;
    uint32_t cdf_size_;

    uint32_t* lut_;

    float* cdf_;

    float integral_;
    float size_;

    float lut_range_;
};

}  // namespace math

#endif
