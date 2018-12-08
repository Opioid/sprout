#include "distribution_3d.hpp"
#include "distribution_1d.inl"
#include "math/vector3.inl"

namespace math {

Distribution_3D::Distribution_3D() = default;

Distribution_3D::~Distribution_3D() {}

void Distribution_3D::init(std::vector<Distribution_2D>& conditional) {
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

Distribution_3D::Continuous Distribution_3D::sample_continuous(float3 const& r3) const {
    auto const w = marginal_.sample_continuous(r3[2]);

    uint32_t const i = static_cast<uint32_t>(w.offset * conditional_size_);
    uint32_t const c = std::min(i, conditional_max_);

    auto const uv = conditional_[c].sample_continuous(r3.xy());

    return {float3(uv.uv, w.offset), uv.pdf * w.pdf};
}

float Distribution_3D::pdf(float3 const& uvw) const {
    float const w_pdf = marginal_.pdf(uvw[2]);

    uint32_t const i = static_cast<uint32_t>(uvw[2] * conditional_size_);
    uint32_t const c = std::min(i, conditional_max_);

    float const uv_pdf = conditional_[c].pdf(uvw.xy());

    return uv_pdf * w_pdf;
}

size_t Distribution_3D::num_bytes() const {
    size_t num_bytes = 0;
    for (auto const& c : conditional_) {
        num_bytes += c.num_bytes();
    }

    return sizeof(*this) + marginal_.num_bytes() + num_bytes;
}

}  // namespace math
