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

	area_ = static_cast<float>(dimensions.x * dimensions.y);
}

math::float2 Distribution_2D::sample_continuous(float u, float v, float& pdf) const {
	math::float2 uv;

	float v_pdf;
	uv.y = marginal_.sample_continuous(v, v_pdf);

	size_t c = static_cast<size_t>(uv.y * static_cast<float>(conditional_.size()));
	float u_pdf;
	uv.x = conditional_[c].sample_continuous(u, u_pdf);

	pdf = u_pdf * v_pdf;// * area_;

	return uv;
}

}

