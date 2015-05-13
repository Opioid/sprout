#pragma once

#include "cdf.hpp"

namespace math {

inline float CDF::init(const std::vector<float>& data) {
	return precompute_1D_pdf_cdf(data);
}

inline uint32_t CDF::sample(float r) const {
	uint32_t idx = binsearch(r);

	float t = (cdf_[idx + 1] - r) / (cdf_[idx + 1] - cdf_[idx]);

	if (t < 1.f) {
		// corner case with r == 0.0f
		float fidx = static_cast<float>(idx);
		idx = static_cast<uint32_t>((1.f - t) * fidx + t * (fidx + 1.f));
	}

	return idx;
}

inline uint32_t CDF::sample(float r, float& p) const {
	uint32_t idx = binsearch(r);

	p = pdf_[idx];

	float t = (cdf_[idx + 1] - r) / (cdf_[idx + 1] - cdf_[idx]);

	if (t < 1.f) {
		// corner case with r == 0.0f
		float fidx = static_cast<float>(idx);
		idx = static_cast<uint32_t>((1.f - t) * fidx + t * (fidx + 1.f));
	}

	return idx;
}

inline float CDF::precompute_1D_pdf_cdf(const std::vector<float>& data) {
	pdf_.resize(data.size());
	cdf_.resize(data.size() + 1);

	float sum = 0.f;

	size_t len = data.size();

	for (size_t i = 0; i < len; ++i) {
		sum += data[i];
	}

	if (0.f == sum) {
		sum = 1.f;
	}

	for (size_t i = 0; i < len; ++i) {
		pdf_[i] = data[i] / sum;
	}

	cdf_[0] = 0.f;

	for (size_t i = 1; i < len; ++i) {
		cdf_[i] = cdf_[i - 1] + pdf_[i - 1];
	}

	cdf_[len] = 1.f;

	return sum;
}

inline uint32_t CDF::binsearch(float v) const {
	//if (v >= 1.f) v = 0.999f; //!!EPSILON; check this
	if (v > 0.999f) {
		v = 0.999f; //!!EPSILON; check this
	}

	uint32_t len = static_cast<uint32_t>(pdf_.size());
	uint32_t idx = len / 2;
	uint32_t si = 0;
	uint32_t ei = len;

	while (v < cdf_[idx] || v > cdf_[idx + 1]) {
		if (v < cdf_[idx]) {
			ei = idx;
		} else if (v > cdf_[idx + 1]) {
			si = idx;
		}

		idx = (si + ei) / 2;
	}

	return idx;
}

}
