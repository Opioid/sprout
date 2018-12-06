#include "distribution_2d.hpp"
#include "distribution_1d.inl"
#include "math/vector2.inl"
#include "thread/thread_pool.hpp"

namespace math {

template <typename T>
void Distribution_t_2D<T>::init(float const* data, int2 dimensions) {
    conditional_.resize(dimensions[1]);

    std::vector<float> integrals(dimensions[1]);

    for (int32_t i = 0; i < dimensions[1]; ++i) {
        conditional_[i].init(data + i * dimensions[0], dimensions[0]);

        integrals[i] = conditional_[i].integral();
    }

    marginal_.init(integrals.data(), dimensions[1]);

    conditional_size_ = static_cast<float>(conditional_.size());
    conditional_max_  = static_cast<uint32_t>(conditional_.size() - 1);
}

template <typename T>
void Distribution_t_2D<T>::init(float const* data, int2 dimensions, thread::Pool& pool) {
    conditional_.resize(dimensions[1]);

    std::vector<float> integrals(static_cast<size_t>(dimensions[1]));

    pool.run_range(
        [this, data, &integrals, dimensions](uint32_t /*id*/, int32_t begin, int32_t end) {
            for (int32_t i = begin; i < end; ++i) {
                conditional_[i].init(data + i * dimensions[0], dimensions[0]);

                integrals[i] = conditional_[i].integral();
            }
        },
        0, dimensions[1]);

    marginal_.init(integrals.data(), dimensions[1]);

    conditional_size_ = static_cast<float>(conditional_.size());
    conditional_max_  = static_cast<uint32_t>(conditional_.size() - 1);
}

template <typename T>
void Distribution_t_2D<T>::init(std::vector<Distribution_impl>& conditional) {
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
typename Distribution_t_2D<T>::Continuous Distribution_t_2D<T>::sample_continuous(float2 r2) const {
    auto const v = marginal_.sample_continuous(r2[1]);

    uint32_t const i = static_cast<uint32_t>(v.offset * conditional_size_);
    uint32_t const c = std::min(i, conditional_max_);

    auto const u = conditional_[c].sample_continuous(r2[0]);

    return {float2(u.offset, v.offset), u.pdf * v.pdf};
}

template <typename T>
float Distribution_t_2D<T>::pdf(float2 uv) const {
    float const v_pdf = marginal_.pdf(uv[1]);

    uint32_t const i = static_cast<uint32_t>(uv[1] * conditional_size_);
    uint32_t const c = std::min(i, conditional_max_);

    float const u_pdf = conditional_[c].pdf(uv[0]);

    return u_pdf * v_pdf;
}

template <typename T>
size_t Distribution_t_2D<T>::num_bytes() const {
    size_t num_bytes = 0;
    for (auto& c : conditional_) {
        num_bytes += c.num_bytes();
    }

    return sizeof(*this) + marginal_.num_bytes() + num_bytes;
}

template class Distribution_t_2D<Distribution_implicit_pdf_lut_lin_1D>;

}  // namespace math
