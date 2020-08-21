#include "distribution_2d.hpp"
#include "distribution_1d.inl"
#include "math/vector2.inl"

namespace math {

Distribution_2D::Distribution_2D() : conditional_size_(0), conditional_(nullptr) {}

Distribution_2D::~Distribution_2D() {
    delete[] conditional_;
}

Distribution_1D* Distribution_2D::allocate(uint32_t num) {
    if (conditional_size_ != num) {
        delete[] conditional_;

        conditional_size_ = num;
        conditional_      = new Distribution_1D[num];
    }

    return conditional_;
}

Distribution_1D* Distribution_2D::conditional() {
    return conditional_;
}

bool Distribution_2D::empty() const {
    return 0 == conditional_size_;
}

void Distribution_2D::init() {
    uint32_t const num_conditional = conditional_size_;

    float* integrals = new float[num_conditional];

    for (uint32_t i = 0; i < num_conditional; ++i) {
        integrals[i] = conditional_[i].integral();
    }

    marginal_.init(integrals, num_conditional);

    conditional_sizef_ = float(num_conditional);
    conditional_max_   = num_conditional - 1;

    delete[] integrals;
}

float Distribution_2D::integral() const {
    return marginal_.integral();
}

typename Distribution_2D::Continuous Distribution_2D::sample_continuous(float2 r2) const {
    auto const v = marginal_.sample_continuous(r2[1]);

    uint32_t const i = uint32_t(v.offset * conditional_sizef_);
    uint32_t const c = std::min(i, conditional_max_);

    auto const u = conditional_[c].sample_continuous(r2[0]);

    return {float2(u.offset, v.offset), u.pdf * v.pdf};
}

float Distribution_2D::pdf(float2 uv) const {
    float const v_pdf = marginal_.pdf(uv[1]);

    uint32_t const i = uint32_t(uv[1] * conditional_sizef_);
    uint32_t const c = std::min(i, conditional_max_);

    float const u_pdf = conditional_[c].pdf(uv[0]);

    return u_pdf * v_pdf;
}

}  // namespace math
