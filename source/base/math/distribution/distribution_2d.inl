#pragma once

#include "distribution_2d.hpp"
#include "distribution_1d.inl"
#include "thread/thread_pool.hpp"

namespace math {

inline void Distribution_2D::init(const float* data, int2 dimensions) {
	conditional_.resize(dimensions.y);

	std::vector<float> integrals(dimensions.y);

	for (int32_t i = 0; i < dimensions.y; ++i) {
		conditional_[i].init(data + i * dimensions.x, dimensions.x);

		integrals[i] = conditional_[i].integral();
	}

	marginal_.init(integrals.data(), dimensions.y);

	conditional_size_ = static_cast<float>(conditional_.size());
	conditional_max_  = static_cast<uint32_t>(conditional_.size() - 1);
}

inline void Distribution_2D::init(const float* data, int2 dimensions, thread::Pool& pool) {
	conditional_.resize(dimensions.y);

	std::vector<float> integrals(dimensions.y);

	pool.run_range([this, data, &integrals, dimensions]
		(uint32_t /*id*/, int32_t begin, int32_t end) {
			for (int32_t i = begin; i < end; ++i) {
				conditional_[i].init(data + i * dimensions.x, dimensions.x);

				integrals[i] = conditional_[i].integral();
			}
	}, 0, dimensions.y);

	marginal_.init(integrals.data(), dimensions.y);

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
	result.y = marginal_.sample_continuous(r2.y, v_pdf);

	uint32_t c = std::min(static_cast<uint32_t>(result.y * conditional_size_), conditional_max_);
	float u_pdf;
	result.x = conditional_[c].sample_continuous(r2.x, u_pdf);

	pdf = u_pdf * v_pdf;

	return result;
}

inline float Distribution_2D::pdf(float2 uv) const {
	float v_pdf = marginal_.pdf(uv.y);

	uint32_t c = std::min(static_cast<uint32_t>(uv.y * conditional_size_), conditional_max_);
	float u_pdf = conditional_[c].pdf(uv.x);

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

