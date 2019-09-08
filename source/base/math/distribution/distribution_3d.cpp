#include "distribution_3d.hpp"
#include "distribution_1d.inl"
#include "math/vector3.inl"

namespace math {

Distribution_3D::Distribution_3D() noexcept : conditional_size_(0), conditional_(nullptr) {}

Distribution_3D::~Distribution_3D() noexcept {
    memory::destroy_aligned(conditional_, conditional_size_);
}

Distribution_2D* Distribution_3D::allocate(uint32_t num) noexcept {
    if (conditional_size_ != num) {
        memory::destroy_aligned(conditional_, conditional_size_);

        conditional_size_ = num;
        conditional_      = memory::construct_array_aligned<Distribution_2D>(num);
    }

    return conditional_;
}

void Distribution_3D::init() noexcept {
    uint32_t const num_conditional = conditional_size_;

    float* integrals = memory::allocate_aligned<float>(num_conditional);

    for (uint32_t i = 0; i < num_conditional; ++i) {
        integrals[i] = conditional_[i].integral();
    }

    marginal_.init(integrals, num_conditional);

    conditional_sizef_ = float(num_conditional);
    conditional_max_   = num_conditional - 1;

    memory::free_aligned(integrals);
}

float Distribution_3D::integral() const noexcept {
    return marginal_.integral();
}

Distribution_3D::Continuous Distribution_3D::sample_continuous(float3 const& r3) const noexcept {
    auto const w = marginal_.sample_continuous(r3[2]);

    uint32_t const i = uint32_t(w.offset * conditional_sizef_);
    uint32_t const c = std::min(i, conditional_max_);

    auto const uv = conditional_[c].sample_continuous(r3.xy());

    return {float3(uv.uv, w.offset), uv.pdf * w.pdf};
}

float Distribution_3D::pdf(float3 const& uvw) const noexcept {
    float const w_pdf = marginal_.pdf(uvw[2]);

    uint32_t const i = uint32_t(uvw[2] * conditional_sizef_);
    uint32_t const c = std::min(i, conditional_max_);

    float const uv_pdf = conditional_[c].pdf(uvw.xy());

    return uv_pdf * w_pdf;
}

size_t Distribution_3D::num_bytes() const noexcept {
    size_t num_bytes = 0;

    for (int32_t i = 0, len = conditional_size_; i < len; ++i) {
        num_bytes += conditional_[i].num_bytes();
    }

    return sizeof(*this) + marginal_.num_bytes() + num_bytes;
}

}  // namespace math
