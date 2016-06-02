#pragma once

#include <vector>
#include <cstddef>

namespace math {

class Distribution_1D {

public:

	void init(const float* data, size_t len);

	float integral() const;

	uint32_t sample_discrete(float r) const;
	uint32_t sample_discrete(float r, float& pdf) const;

	float sample_continuous(float r, float& pdf) const;

	float pdf(float u) const;

private:

	void precompute_1D_pdf_cdf(const float* data, size_t len);

	std::vector<float> pdf_;
	std::vector<float> cdf_;

	float integral_;
	float size_;
};

// LUT implementation inspired by
// https://geidav.wordpress.com/2013/12/29/optimizing-binary-search/

class Distribution_lut_1D {

public:

	void init(const float* data, uint32_t len, uint32_t lut_size = 0);

	float integral() const;

	uint32_t sample_discrete(float r) const;
	uint32_t sample_discrete(float r, float& pdf) const;

	float sample_continuous(float r, float& pdf) const;

	float pdf(float u) const;

private:

	uint32_t map(float s) const;

	void precompute_1D_pdf_cdf(const float* data, uint32_t len);
	void init_lut(uint32_t lut_size);

	uint32_t lut_heuristic(uint32_t len) const;

	std::vector<uint32_t> lut_;
	std::vector<float> pdf_;
	std::vector<float> cdf_;

	float integral_;
	float size_;

	float lut_range_;
};

}
