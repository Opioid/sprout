#pragma once

#include "distribution_1d.hpp"
#include <algorithm>

namespace math {

inline void Distribution_1D::init(const float* data, size_t len) {
	precompute_1D_pdf_cdf(data, len);
	size_ = static_cast<float>(len);
}

inline float Distribution_1D::integral() const {
	return integral_;
}

inline uint32_t Distribution_1D::sample_discrete(float r) const {
	auto it = std::lower_bound(cdf_.begin(), cdf_.end(), r);

	uint32_t offset =  0;
	if (it != cdf_.begin()) {
		offset = static_cast<uint32_t>(it - cdf_.begin() - 1);
	}

	return offset;
}

inline uint32_t Distribution_1D::sample_discrete(float r, float& pdf) const {
	uint32_t offset = sample_discrete(r);

	pdf = pdf_[offset];

	return offset;
}

inline float Distribution_1D::sample_continuous(float r, float& pdf) const {
	uint32_t offset = sample_discrete(r);

	pdf = pdf_[offset];

	float c = cdf_[offset + 1];
	float t = (c - r) / (c - cdf_[offset]);

	return (static_cast<float>(offset) + t) / size_;
}

inline float Distribution_1D::pdf(float u) const {
	uint32_t offset = static_cast<uint32_t>(u * size_);

	return pdf_[offset];
}

inline void Distribution_1D::precompute_1D_pdf_cdf(const float* data, size_t len) {
	pdf_.resize(len);
	cdf_.resize(len + 1);

	float integral = 0.f;
	for (size_t i = 0; i < len; ++i) {
		integral += data[i];
	}

	if (0.f == integral) {
		integral = 1.f;
	}

	for (size_t i = 0; i < len; ++i) {
		pdf_[i] = data[i] / integral;
	}

	cdf_[0] = 0.f;
	for (size_t i = 1; i < len; ++i) {
		cdf_[i] = cdf_[i - 1] + pdf_[i - 1];
	}
	cdf_[len] = 1.f;

	integral_ = integral;
}

}
