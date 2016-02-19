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

	size_ = static_cast<float>(len);
}

template<uint32_t LUT_bits>
Distribution_lut_1D<LUT_bits>::Distribution_lut_1D() :
	lut_((1 << LUT_bits) + 2),
	lut_range_(static_cast<float>(1 << LUT_bits)) {}

template<uint32_t LUT_bits>
void Distribution_lut_1D<LUT_bits>::init(const float* data, size_t len) {
	precompute_1D_pdf_cdf(data, len);
	init_lut();
}

template<uint32_t LUT_bits>
float Distribution_lut_1D<LUT_bits>::integral() const {
	return integral_;
}

template<uint32_t LUT_bits>
uint32_t Distribution_lut_1D<LUT_bits>::sample_discrete(float r) const {
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

template<uint32_t LUT_bits>
uint32_t Distribution_lut_1D<LUT_bits>::sample_discrete(float r, float& pdf) const {
	uint32_t offset = sample_discrete(r);

	pdf = pdf_[offset];

	return offset;
}

template<uint32_t LUT_bits>
float Distribution_lut_1D<LUT_bits>::sample_continuous(float r, float& pdf) const {
	uint32_t offset = sample_discrete(r);

	pdf = pdf_[offset];

	float c = cdf_[offset + 1];
	float t = (c - r) / (c - cdf_[offset]);

	return (static_cast<float>(offset) + t) / size_;
}

template<uint32_t LUT_bits>
float Distribution_lut_1D<LUT_bits>::pdf(float u) const {
	uint32_t offset = static_cast<uint32_t>(u * size_);

	return pdf_[offset];
}

template<uint32_t LUT_bits>
uint32_t Distribution_lut_1D<LUT_bits>::map(float s) const {
	return static_cast<uint32_t>(s * lut_range_);
}

template<uint32_t LUT_bits>
void Distribution_lut_1D<LUT_bits>::precompute_1D_pdf_cdf(const float* data, size_t len) {
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

template<uint32_t LUT_bits>
void Distribution_lut_1D<LUT_bits>::init_lut() {
	lut_[0] = 0;

	uint32_t border = 0;
	uint32_t last = 0;

	for (size_t i = 1, len = cdf_.size(); i < len; ++i) {
		uint32_t mapped = map(cdf_[i]);

		if (mapped > border) {
			last = static_cast<uint32_t>(i);

			for (size_t j = border + 1; j <= mapped; ++j) {
				lut_[j] = last;
			}

			border = mapped;
		}
	}

	for (size_t i = border + 1, len = lut_.size(); i < len; ++i) {
		lut_[i] = last;
	}
}

/*
inline void Distribution_luty_1D::init(const float* data, size_t len) {
	precompute_1D_pdf_cdf(data, len);
	init_lut(8);
}

inline float Distribution_luty_1D::integral() const {
	return integral_;
}

inline uint32_t Distribution_luty_1D::sample_discrete(float r) const {
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

inline uint32_t Distribution_luty_1D::sample_discrete(float r, float& pdf) const {
	uint32_t offset = sample_discrete(r);

	pdf = pdf_[offset];

	return offset;
}

inline float Distribution_luty_1D::sample_continuous(float r, float& pdf) const {
	uint32_t offset = sample_discrete(r);

	pdf = pdf_[offset];

	float c = cdf_[offset + 1];
	float t = (c - r) / (c - cdf_[offset]);

	return (static_cast<float>(offset) + t) / size_;
}

inline float Distribution_luty_1D::pdf(float u) const {
	uint32_t offset = static_cast<uint32_t>(u * size_);

	return pdf_[offset];
}

inline uint32_t Distribution_luty_1D::map(float s) const {
	return static_cast<uint32_t>(s * lut_range_);
}

inline void Distribution_luty_1D::precompute_1D_pdf_cdf(const float* data, size_t len) {
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

inline void Distribution_luty_1D::init_lut(uint32_t lut_bits) {
	lut_.resize((1 << lut_bits) + 2),
	lut_range_ = static_cast<float>(1 << lut_bits);

	lut_[0] = 0;

	uint32_t border = 0;
	uint32_t last = 0;

	for (size_t i = 1, len = cdf_.size(); i < len; ++i) {
		uint32_t mapped = map(cdf_[i]);

		if (mapped > border) {
			last = static_cast<uint32_t>(i);

			for (size_t j = border + 1; j <= mapped; ++j) {
				lut_[j] = last;
			}

			border = mapped;
		}
	}

	for (size_t i = border + 1, len = lut_.size(); i < len; ++i) {
		lut_[i] = last;
	}
}
*/

}
