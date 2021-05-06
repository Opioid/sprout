#include "distribution_3d.hpp"
#include "distribution_1d.inl"
#include "math/vector4.inl"

namespace math {

Distribution_3D::Distribution_3D() : conditional_size_(0), conditional_(nullptr) {}

Distribution_3D::~Distribution_3D() {
    delete[] conditional_;
}

Distribution_2D* Distribution_3D::allocate(uint32_t num) {
    if (conditional_size_ != num) {
        delete[] conditional_;

        conditional_size_ = num;
        conditional_      = new Distribution_2D[num];
    }

    return conditional_;
}

void Distribution_3D::init() {
    uint32_t const num_conditional = conditional_size_;

    float* integrals = new float[num_conditional];

    for (uint32_t i = 0; i < num_conditional; ++i) {
        integrals[i] = conditional_[i].integral();
    }

    marginal_.init(integrals, num_conditional);

    conditional_sizef_ = float(num_conditional);

    delete[] integrals;
}

float Distribution_3D::integral() const {
    return marginal_.integral();
}

float4 Distribution_3D::sample_continuous(float3_p r3) const {
    auto const w = marginal_.sample_continuous(r3[2]);

    uint32_t const i = uint32_t(w.offset * conditional_sizef_);
    uint32_t const c = std::min(i, conditional_size_ - 1);

    auto const uv = conditional_[c].sample_continuous(r3.xy());

    return float4(uv.uv, w.offset, uv.pdf * w.pdf);
}

float Distribution_3D::pdf(float3_p uvw) const {
    float const w_pdf = marginal_.pdf(uvw[2]);

    uint32_t const i = uint32_t(uvw[2] * conditional_sizef_);
    uint32_t const c = std::min(i, conditional_size_ - 1);

    float const uv_pdf = conditional_[c].pdf(uvw.xy());

    return uv_pdf * w_pdf;
}

}  // namespace math
