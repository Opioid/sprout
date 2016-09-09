#pragma once

#include "distribution_1d.hpp"
#include <algorithm>

namespace math {

inline void Distribution_1D::init(const float* data, size_t len) {
	precompute_1D_pdf_cdf(data, len);
}

inline float Distribution_1D::integral() const {
	return integral_;
}

inline uint32_t Distribution_1D::sample_discrete(float r) const {
	auto it = std::lower_bound(cdf_.begin(), cdf_.end(), r);

	uint32_t offset = 0;
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

inline float Distribution_1D::pdf(uint32_t index) const {
	return pdf_[index];
}

inline float Distribution_1D::pdf(float u) const {
	uint32_t offset = static_cast<uint32_t>(u * size_);

	return pdf_[offset];
}

inline size_t Distribution_1D::num_bytes() const {
	return sizeof(*this) + sizeof(float) * (pdf_.size() + cdf_.size());
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

	size_ = static_cast<float>(len);
}

inline void Distribution_lut_1D::init(const float* data, uint32_t len, uint32_t lut_size) {
	precompute_1D_pdf_cdf(data, len);

	if (0 == lut_size) {
		lut_size = lut_heuristic(len);
	}

	init_lut(lut_size);
}

inline float Distribution_lut_1D::integral() const {
	return integral_;
}

inline uint32_t Distribution_lut_1D::sample_discrete(float r) const {
	uint32_t bucket = map(r);

	uint32_t begin = lut_[bucket];
	uint32_t end   = lut_[bucket + 1];

	auto it = std::lower_bound(cdf_.begin() + begin, cdf_.begin() + end, r);

	uint32_t offset =  0;
	if (it != cdf_.begin()) {
		offset = static_cast<uint32_t>(it - cdf_.begin() - 1);
	}

//	auto ito = std::lower_bound(cdf_.begin(), cdf_.end(), r);
//	if (it != ito) {
//		std::cout << "Larmy larm" << std::endl;
//	}

	return offset;
}

inline uint32_t Distribution_lut_1D::sample_discrete(float r, float& pdf) const {
	uint32_t offset = sample_discrete(r);

	pdf = pdf_[offset];

	return offset;
}

inline float Distribution_lut_1D::sample_continuous(float r, float& pdf) const {
	uint32_t offset = sample_discrete(r);

	pdf = pdf_[offset];

	float c = cdf_[offset + 1];
	float t = (c - r) / (c - cdf_[offset]);

	return (static_cast<float>(offset) + t) / size_;
}

inline float Distribution_lut_1D::pdf(uint32_t index) const {
	return pdf_[index];
}

inline float Distribution_lut_1D::pdf(float u) const {
	uint32_t offset = static_cast<uint32_t>(u * size_);

	return pdf_[offset];
}

inline size_t Distribution_lut_1D::num_bytes() const {
	return sizeof(*this) +
		   sizeof(float) * (pdf_.size() + cdf_.size()) +
		   sizeof(uint32_t) * lut_.size();
}

inline uint32_t Distribution_lut_1D::map(float s) const {
	return static_cast<uint32_t>(s * lut_range_);
}

inline void Distribution_lut_1D::precompute_1D_pdf_cdf(const float* data, uint32_t len) {
	pdf_.resize(len);
	cdf_.resize(len + 1);

	float integral = 0.f;
	for (uint32_t i = 0; i < len; ++i) {
		integral += data[i];
	}

	if (0.f == integral) {
		integral = 1.f;
	}

	for (uint32_t i = 0; i < len; ++i) {
		pdf_[i] = data[i] / integral;
	}

	cdf_[0] = 0.f;
	for (uint32_t i = 1; i < len; ++i) {
		cdf_[i] = cdf_[i - 1] + pdf_[i - 1];
	}
	cdf_[len] = 1.f;

	integral_ = integral;

	size_ = static_cast<float>(len);
}

inline void Distribution_lut_1D::init_lut(uint32_t lut_size) {
	lut_.resize(lut_size + 2),
	lut_range_ = static_cast<float>(lut_size);

	lut_[0] = 0;

	uint32_t border = 0;
	uint32_t last = 0;

	for (uint32_t i = 1, len = static_cast<uint32_t>(cdf_.size()); i < len; ++i) {
		uint32_t mapped = map(cdf_[i]);

		if (mapped > border) {
			last = i;

			for (uint32_t j = border + 1; j <= mapped; ++j) {
				lut_[j] = last;
			}

			border = mapped;
		}
	}

	for (uint32_t i = border + 1, len = static_cast<uint32_t>(lut_.size()); i < len; ++i) {
		lut_[i] = last;
	}
}

inline uint32_t Distribution_lut_1D::lut_heuristic(uint32_t len) const {
	return std::max(len / 16, 1u);
}

}
