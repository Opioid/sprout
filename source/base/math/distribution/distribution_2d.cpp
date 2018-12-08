#include "distribution_2d.hpp"
#include "distribution_1d.inl"
#include "math/vector2.inl"

namespace math {

template <typename T>
Distribution_t_2D<T>::Distribution_t_2D() noexcept = default;

template <typename T>
Distribution_t_2D<T>::~Distribution_t_2D() noexcept {}

template <typename T>
void Distribution_t_2D<T>::init(std::vector<Distribution_impl>& conditional) noexcept {
    conditional_ = std::move(conditional);

    std::vector<float> integrals(conditional_.size());

    uint32_t const num_conditional = static_cast<uint32_t>(conditional_.size());

    for (uint32_t i = 0; i < num_conditional; ++i) {
        integrals[i] = conditional_[i].integral();
    }

    marginal_.init(integrals.data(), num_conditional);

    conditional_size_ = static_cast<float>(num_conditional);
    conditional_max_  = num_conditional - 1;
}

template <typename T>
float Distribution_t_2D<T>::integral() const noexcept {
    return marginal_.integral();
}

template <typename T>
typename Distribution_t_2D<T>::Continuous Distribution_t_2D<T>::sample_continuous(float2 r2) const
    noexcept {
    auto const v = marginal_.sample_continuous(r2[1]);

    uint32_t const i = static_cast<uint32_t>(v.offset * conditional_size_);
    uint32_t const c = std::min(i, conditional_max_);

    auto const u = conditional_[c].sample_continuous(r2[0]);

    return {float2(u.offset, v.offset), u.pdf * v.pdf};
}

template <typename T>
float Distribution_t_2D<T>::pdf(float2 uv) const noexcept {
    float const v_pdf = marginal_.pdf(uv[1]);

    uint32_t const i = static_cast<uint32_t>(uv[1] * conditional_size_);
    uint32_t const c = std::min(i, conditional_max_);

    float const u_pdf = conditional_[c].pdf(uv[0]);

    return u_pdf * v_pdf;
}

template <typename T>
size_t Distribution_t_2D<T>::num_bytes() const noexcept {
    size_t num_bytes = 0;
    for (auto const& c : conditional_) {
        num_bytes += c.num_bytes();
    }

    return sizeof(*this) + marginal_.num_bytes() + num_bytes;
}

template class Distribution_t_2D<Distribution_implicit_pdf_lut_lin_1D>;

}  // namespace math
