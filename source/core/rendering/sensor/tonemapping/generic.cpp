#include "generic.hpp"
#include "base/math/vector.inl"

namespace rendering { namespace sensor { namespace tonemapping {

Generic::Generic(float contrast, float shoulder, float mid_in, float mid_out, float hdr_max,
				 float exposure) :
	a_(contrast),
	d_(shoulder),
	exposure_factor_(std::exp2(exposure)) {
	float mid_in_pow_a = std::pow(mid_in, a_);
	float hdr_max_pow_a = std::pow(hdr_max, a_);
	float hdr_max_pow_a_pow_d = std::pow(hdr_max_pow_a, d_);

	float y = std::pow(mid_in_pow_a, d_) * mid_out;
	float z = hdr_max_pow_a_pow_d - y;

	b_ = (-mid_in_pow_a + hdr_max_pow_a * mid_out) / z;
	c_ = (hdr_max_pow_a_pow_d * mid_in_pow_a - hdr_max_pow_a * y) / z;
}

float3 Generic::tonemap(float3_p color) const {
	return tonemap_function(exposure_factor_ * color);
}

float3 Generic::tonemap_function(float3_p color) const {

	float mx = tonemap_function(color.x);

	float my = tonemap_function(color.y);

	return float3(mx,
				  my,
				  tonemap_function(color.z));
}

float Generic::tonemap_function(float x) const {
	float x_pow_a = std::pow(x, a_);
	return x_pow_a / (std::pow(x_pow_a, d_) * b_ + c_);
}

}}}
