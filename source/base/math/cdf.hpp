#pragma once

#include <vector>

namespace math {

class CDF {
public:

	float init(const std::vector<float>& data);

	uint32_t sample(float r) const;
	uint32_t sample(float r, float& p) const;

private:

	float precompute_1D_pdf_cdf(const std::vector<float>& data);

	uint32_t binsearch(float v) const;

	std::vector<float> pdf_;
	std::vector<float> cdf_;
};

}
