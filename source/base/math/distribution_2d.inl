#pragma once

#include "distribution_2d.hpp"
#include "distribution_1d.inl"

namespace math {

void Distribution_2D::init(const float* data, const math::uint2& dimensions) {
	conditional_.resize(dimensions.y);

	std::vector<float> integrals(dimensions.y);

	for (uint32_t i = 0; i < dimensions.y; ++i) {
		conditional_[i].init(data + i * dimensions.x, dimensions.x);

		integrals[i] = conditional_[i].integral();
	}

	marginal_.init(integrals.data(), dimensions.y);
}

math::float2 Distribution_2D::sample_continuous(math::float2 r2, float& pdf) const {
	math::float2 result;

	float v_pdf;
	result.y = marginal_.sample_continuous(r2.y, v_pdf);

	size_t c = static_cast<size_t>(result.y * static_cast<float>(conditional_.size() - 1));
	float u_pdf;
	result.x = conditional_[c].sample_continuous(r2.x, u_pdf);

	pdf = u_pdf * v_pdf;

	return result;
}

float Distribution_2D::pdf(math::float2 uv) const {
	float v_pdf = marginal_.pdf(uv.y);

	size_t c = static_cast<size_t>(uv.y * static_cast<float>(conditional_.size() - 1));
	float u_pdf = conditional_[c].pdf(uv.x);

	return u_pdf * v_pdf;
}

}

