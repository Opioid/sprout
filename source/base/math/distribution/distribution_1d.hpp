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

template<uint32_t LUT_bits>
class Distribution_lut_1D {
public:

	Distribution_lut_1D();

	void init(const float* data, size_t len);

	float integral() const;

	uint32_t sample_discrete(float r) const;
	uint32_t sample_discrete(float r, float& pdf) const;

	float sample_continuous(float r, float& pdf) const;

	float pdf(float u) const;

private:

	uint32_t map(float s) const;

	void precompute_1D_pdf_cdf(const float* data, size_t len);
	void init_lut();

	std::vector<uint32_t> lut_;
	std::vector<float> pdf_;
	std::vector<float> cdf_;

	float integral_;
	float size_;

	const float lut_range_;
};

}
