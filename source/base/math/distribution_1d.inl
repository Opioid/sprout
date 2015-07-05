#pragma once

#include "distribution_1d.hpp"
#include <algorithm>

namespace math {

inline void Distribution_1D::init(const float* data, size_t len) {
	size_ = static_cast<float>(len);
	precompute_1D_pdf_cdf(data, len);
}

inline float Distribution_1D::integral() const {
	return integral_;
}

inline float Distribution_1D::sample_continuous(float r, float& pdf) const {
	auto it = std::lower_bound(cdf_.begin(), cdf_.end(), r);

	uint32_t offset =  0;
	if (it != cdf_.begin()) {
		offset = static_cast<uint32_t>(it - cdf_.begin() - 1);
	}

	pdf = pdf_[offset];

	float t = (cdf_[offset + 1] - r) / (cdf_[offset + 1] - cdf_[offset]);

	return (static_cast<float>(offset) + t) / size_;
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
	auto it = std::lower_bound(cdf_.begin(), cdf_.end(), r);

	uint32_t offset =  0;
	if (it != cdf_.begin()) {
		offset = static_cast<uint32_t>(it - cdf_.begin() - 1);
	}

	pdf = pdf_[offset];

	return offset;
}

inline void Distribution_1D::precompute_1D_pdf_cdf(const float* data, size_t len) {
	pdf_.resize(len);
	cdf_.resize(len + 1);

	integral_ = 0.f;
	for (size_t i = 0; i < len; ++i) {
		integral_ += data[i];
	}

	if (0.f == integral_) {
		integral_ = 1.f;
	}

	for (size_t i = 0; i < len; ++i) {
		pdf_[i] = data[i] / integral_;
	}

	cdf_[0] = 0.f;
	for (size_t i = 1; i < len; ++i) {
		cdf_[i] = cdf_[i - 1] + pdf_[i - 1];
	}
	cdf_[len] = 1.f;
}

}
