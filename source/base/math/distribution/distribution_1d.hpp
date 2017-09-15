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

	float pdf(uint32_t index) const;
	float pdf(float u) const;

	size_t num_bytes() const;

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

	float pdf(uint32_t index) const;
	float pdf(float u) const;

	size_t num_bytes() const;

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

class Distribution_implicit_pdf_lut_1D {

public:

	Distribution_implicit_pdf_lut_1D();
	~Distribution_implicit_pdf_lut_1D();

	void init(const float* data, uint32_t len, uint32_t lut_bucket_size = 0);

	float integral() const;

	uint32_t sample_discrete(float r) const;
	uint32_t sample_discrete(float r, float& pdf) const;

	float sample_continuous(float r, float& pdf) const;

	float pdf(uint32_t index) const;
	float pdf(float u) const;

	uint32_t lut_size() const;

	size_t num_bytes() const;

private:

	uint32_t map(float s) const;

	void precompute_1D_pdf_cdf(const float* data, uint32_t len);
	void init_lut(uint32_t lut_size);

	uint32_t lut_heuristic(uint32_t len) const;

	uint32_t* lut_;
	uint32_t  lut_size_;

	float*   cdf_;
	uint32_t cdf_size_;

	float integral_;
	float size_;

	float lut_range_;
};

}
