#pragma once

#include "distribution_2d.hpp"
#include "distribution_1d.inl"

namespace math {

inline void Distribution_2D::init(const float* data, int2 dimensions) {
	conditional_.resize(dimensions.y);

	std::vector<float> integrals(dimensions.y);

	for (int32_t i = 0; i < dimensions.y; ++i) {
		conditional_[i].init(data + i * dimensions.x, dimensions.x);

		integrals[i] = conditional_[i].integral();
	}

	marginal_.init(integrals.data(), dimensions.y);

	conditional_max_ = static_cast<float>(conditional_.size() - 1);
}

inline float2 Distribution_2D::sample_continuous(float2 r2, float& pdf) const {
	float2 result;

	float v_pdf;
	result.y = marginal_.sample_continuous(r2.y, v_pdf);

	size_t c = static_cast<size_t>(result.y * conditional_max_);
	float u_pdf;
	result.x = conditional_[c].sample_continuous(r2.x, u_pdf);

	pdf = u_pdf * v_pdf;

	return result;
}

inline float Distribution_2D::pdf(float2 uv) const {
	float v_pdf = marginal_.pdf(uv.y);

	size_t c = static_cast<size_t>(uv.y * conditional_max_);
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

