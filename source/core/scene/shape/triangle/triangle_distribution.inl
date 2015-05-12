#pragma once

#include "triangle_distribution.hpp"
#include "triangle_primitive.inl"

namespace scene { namespace shape { namespace triangle {

inline void Distribution::init(uint32_t part, const std::vector<Triangle>& triangles, const math::float3& scale) {
	std::vector<float> areas;

	triangle_mapping_.clear();

	uint32_t i = 0;
	for (auto& t : triangles) {
		if (t.material_index == part) {
			areas.push_back(t.area(scale));
			triangle_mapping_.push_back(i);
		}

		++i;
	}

	area_ = precompute_1D_pdf_cdf(areas);
}

inline float Distribution::area() const {
	return area_;
}

inline uint32_t Distribution::sample(float r) const {
/*	float len = static_cast<float>(pdf_.size());
	uint32_t index = static_cast<uint32_t>(len * r - 0.001f);
	return index;
*/

	uint32_t idx = binsearch(r);

	float t = (cdf_[idx + 1] - r) / (cdf_[idx + 1] - cdf_[idx]);

	if (t < 1.f) {
		// corner case with r == 0.0f
		float fidx = static_cast<float>(idx);
		idx = static_cast<uint32_t>((1.f - t) * fidx + t * (fidx + 1.f));
	}

	return triangle_mapping_[idx];
}

inline uint32_t Distribution::binsearch(float v) const {
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

inline float Distribution::precompute_1D_pdf_cdf(const std::vector<float>& data) {
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

}}}
