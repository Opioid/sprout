#pragma once

#include "distribution_1d.hpp"
#include "math/vector.hpp"

namespace thread { class Pool; }

namespace math {

class Distribution_2D {

public:

//	using Distribution_impl = Distribution_1D;
//	using Distribution_impl = Distribution_lut_1D;
	using Distribution_impl = Distribution_implicit_pdf_lut_1D;

	void init(const float* data, int2 dimensions);
	void init(const float* data, int2 dimensions, thread::Pool& pool);
	void init(std::vector<Distribution_impl>& conditional);

	float2 sample_continuous(float2 r2, float& pdf) const;

	float pdf(float2 uv) const;

	size_t num_bytes() const;

private:

	Distribution_impl marginal_;

	std::vector<Distribution_impl> conditional_;

	float conditional_size_;
	uint32_t conditional_max_;
};

}
