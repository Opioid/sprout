#pragma once

#include <vector>
#include <cstddef>

namespace math {

class Distribution_1D {
public:

	void init(const float* data, size_t len);

	float integral() const;

	float sample_continuous(float r, float& pdf) const;

	uint32_t sample_discrete(float r) const;
	uint32_t sample_discrete(float r, float& pdf) const;

	float pdf(float u) const;

private:

	void precompute_1D_pdf_cdf(const float* data, size_t len);

	std::vector<float> pdf_;
	std::vector<float> cdf_;

	float integral_;
	float size_;
};

}
