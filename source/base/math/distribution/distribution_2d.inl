#pragma once

#include "distribution_2d.hpp"
#include "distribution_1d.inl"
#include "thread/thread_pool.hpp"

namespace math {

inline void Distribution_2D::init(const float* data, int2 dimensions) {
	conditional_.resize(dimensions.v[1]);

	std::vector<float> integrals(dimensions.v[1]);

	for (int32_t i = 0; i < dimensions.v[1]; ++i) {
		conditional_[i].init(data + i * dimensions.v[0], dimensions.v[0]);

		integrals[i] = conditional_[i].integral();
	}

	marginal_.init(integrals.data(), dimensions.v[1]);

	conditional_size_ = static_cast<float>(conditional_.size());
	conditional_max_  = static_cast<uint32_t>(conditional_.size() - 1);
}

inline void Distribution_2D::init(const float* data, int2 dimensions, thread::Pool& pool) {
	conditional_.resize(dimensions.v[1]);

	std::vector<float> integrals(dimensions.v[1]);

	pool.run_range([this, data, &integrals, dimensions]
		(uint32_t /*id*/, int32_t begin, int32_t end) {
			for (int32_t i = begin; i < end; ++i) {
				conditional_[i].init(data + i * dimensions.v[0], dimensions.v[0]);

				integrals[i] = conditional_[i].integral();
			}
	}, 0, dimensions.v[1]);

	marginal_.init(integrals.data(), dimensions.v[1]);

	conditional_size_ = static_cast<float>(conditional_.size());
	conditional_max_  = static_cast<uint32_t>(conditional_.size() - 1);
}

inline void Distribution_2D::init(std::vector<Distribution_impl>& conditional) {
	conditional_ = std::move(conditional);

	std::vector<float> integrals(conditional_.size());

	uint32_t num_conditional = static_cast<uint32_t>(conditional_.size());

	for (uint32_t i = 0; i < num_conditional; ++i) {
		integrals[i] = conditional_[i].integral();
	}

	marginal_.init(integrals.data(), num_conditional);

	conditional_size_ = static_cast<float>(num_conditional);
	conditional_max_  = num_conditional - 1;
}

inline float2 Distribution_2D::sample_continuous(float2 r2, float& pdf) const {
	float2 result;

	float v_pdf;
	result.v[1] = marginal_.sample_continuous(r2.v[1], v_pdf);

	uint32_t c = std::min(static_cast<uint32_t>(result.v[1] * conditional_size_), conditional_max_);
	float u_pdf;
	result.v[0] = conditional_[c].sample_continuous(r2.v[0], u_pdf);

	pdf = u_pdf * v_pdf;

	return result;
}

inline float Distribution_2D::pdf(float2 uv) const {
	float v_pdf = marginal_.pdf(uv.v[1]);

	uint32_t c = std::min(static_cast<uint32_t>(uv.v[1] * conditional_size_), conditional_max_);
	float u_pdf = conditional_[c].pdf(uv.v[0]);

	return u_pdf * v_pdf;
}

inline size_t Distribution_2D::num_bytes() const {
	size_t num_bytes = 0;
	for (auto& c : conditional_) {
		num_bytes += c.num_bytes();
	}

	return sizeof(*this) + marginal_.num_bytes() + num_bytes;
}

}

