#include "distribution_2d.hpp"
#include "distribution_1d.inl"
#include "math/vector2.inl"

namespace math {

template <typename T>
Distribution_t_2D<T>::Distribution_t_2D() : conditional_size_(0), conditional_(nullptr) {}

template <typename T>
Distribution_t_2D<T>::~Distribution_t_2D() {
    delete[] conditional_;
}

template <typename T>
typename Distribution_t_2D<T>::Distribution_impl* Distribution_t_2D<T>::allocate(uint32_t num) {
    if (conditional_size_ != num) {
        delete[] conditional_;

        conditional_size_ = num;
        conditional_      = new Distribution_impl[num];
    }

    return conditional_;
}

template <typename T>
typename Distribution_t_2D<T>::Distribution_impl* Distribution_t_2D<T>::conditional() {
    return conditional_;
}

template <typename T>
bool Distribution_t_2D<T>::empty() const {
    return 0 == conditional_size_;
}

template <typename T>
void Distribution_t_2D<T>::init() {
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

template <typename T>
float Distribution_t_2D<T>::integral() const {
    return marginal_.integral();
}

template <typename T>
typename Distribution_t_2D<T>::Continuous Distribution_t_2D<T>::sample_continuous(float2 r2) const {
    auto const v = marginal_.sample_continuous(r2[1]);

    uint32_t const i = uint32_t(v.offset * conditional_sizef_);
    uint32_t const c = std::min(i, conditional_max_);

    auto const u = conditional_[c].sample_continuous(r2[0]);

    return {float2(u.offset, v.offset), u.pdf * v.pdf};
}

template <typename T>
float Distribution_t_2D<T>::pdf(float2 uv) const {
    float const v_pdf = marginal_.pdf(uv[1]);

    uint32_t const i = uint32_t(uv[1] * conditional_sizef_);
    uint32_t const c = std::min(i, conditional_max_);

    float const u_pdf = conditional_[c].pdf(uv[0]);

    return u_pdf * v_pdf;
}

template class Distribution_t_2D<Distribution_implicit_pdf_lut_lin_1D>;

}  // namespace math
