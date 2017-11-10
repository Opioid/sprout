#ifndef SU_BASE_MATH_DISTRIBUTION_DISTRIBUTION_2D_INL
#define SU_BASE_MATH_DISTRIBUTION_DISTRIBUTION_2D_INL

#include "distribution_2d.hpp"
#include "distribution_1d.inl"
#include "math/vector2.inl"
#include "thread/thread_pool.hpp"

namespace math {

template<typename T>
void Distribution_t_2D<T>::init(const float* data, int2 dimensions) {
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

template<typename T>
void Distribution_t_2D<T>::init(const float* data, int2 dimensions, thread::Pool& pool) {
	conditional_.resize(dimensions[1]);

	std::vector<float> integrals(dimensions[1]);

	pool.run_range([this, data, &integrals, dimensions]
		(uint32_t /*id*/, int32_t begin, int32_t end) {
			for (int32_t i = begin; i < end; ++i) {
				conditional_[i].init(data + i * dimensions[0], dimensions[0]);

				integrals[i] = conditional_[i].integral();
			}
	}, 0, dimensions[1]);

	marginal_.init(integrals.data(), dimensions[1]);

	conditional_size_ = static_cast<float>(conditional_.size());
	conditional_max_  = static_cast<uint32_t>(conditional_.size() - 1);
}

template<typename T>
void Distribution_t_2D<T>::init(std::vector<Distribution_impl>& conditional) {
	conditional_ = std::move(conditional);

	std::vector<float> integrals(conditional_.size());

	const uint32_t num_conditional = static_cast<uint32_t>(conditional_.size());

	for (uint32_t i = 0; i < num_conditional; ++i) {
		integrals[i] = conditional_[i].integral();
	}

	marginal_.init(integrals.data(), num_conditional);

	conditional_size_ = static_cast<float>(num_conditional);
	conditional_max_  = num_conditional - 1;
}

template<typename T>
float2 Distribution_t_2D<T>::sample_continuous(float2 r2, float& pdf) const {
	float2 result;

	float v_pdf;
	result[1] = marginal_.sample_continuous(r2[1], v_pdf);

	const uint32_t i = static_cast<uint32_t>(result[1] * conditional_size_);
	const uint32_t c = std::min(i, conditional_max_);

	float u_pdf;
	result[0] = conditional_[c].sample_continuous(r2[0], u_pdf);

	pdf = u_pdf * v_pdf;

	return result;
}

template<typename T>
float Distribution_t_2D<T>::pdf(float2 uv) const {
	const float v_pdf = marginal_.pdf(uv[1]);

	const uint32_t i = static_cast<uint32_t>(uv[1] * conditional_size_);
	const uint32_t c = std::min(i, conditional_max_);
	const float u_pdf = conditional_[c].pdf(uv[0]);

	return u_pdf * v_pdf;
}

template<typename T>
size_t Distribution_t_2D<T>::num_bytes() const {
	size_t num_bytes = 0;
	for (auto& c : conditional_) {
		num_bytes += c.num_bytes();
	}

	return sizeof(*this) + marginal_.num_bytes() + num_bytes;
}

}

#endif
