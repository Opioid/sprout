#pragma once

#include "base/math/vector.hpp"

namespace scene { namespace shape { namespace triangle {

struct Triangle;

class Distribution {
public:

	void init(uint32_t part, const std::vector<Triangle>& triangles, const math::float3& scale);

	float area() const;

	uint32_t sample(float r) const;

private:

	uint32_t binsearch(float v) const;

	float precompute_1D_pdf_cdf(const std::vector<float>& data);

	float area_;

	std::vector<float> pdf_;
	std::vector<float> cdf_;

	std::vector<uint32_t> triangle_mapping_;
};

}}}
