#include "generic.hpp"
#include "image/typed_image.inl"
#include "base/math/vector4.inl"

namespace rendering { namespace postprocessor { namespace tonemapping {

Generic::Generic(float contrast, float shoulder, float mid_in, float mid_out, float hdr_max) :
	a_(contrast),
	d_(shoulder),
	hdr_max_(hdr_max) {
	float const ad = contrast * shoulder;

	float const midi_pow_a  = std::pow(mid_in, contrast);
	float const midi_pow_ad = std::pow(mid_in, ad);
	float const hdrm_pow_a  = std::pow(hdr_max, contrast);
	float const hdrm_pow_ad = std::pow(hdr_max, ad);

	float const u = hdrm_pow_ad * mid_out - midi_pow_ad * mid_out;
	float const v = midi_pow_ad * mid_out;

	b_ = -((-midi_pow_a + (mid_out * (hdrm_pow_ad * midi_pow_a - hdrm_pow_a * v)) / u) / v);
	c_ = (hdrm_pow_ad * midi_pow_a - hdrm_pow_a * v) / u;
}

void Generic::apply(uint32_t /*id*/, uint32_t /*pass*/, int32_t begin, int32_t end,
					const image::Float4& source, image::Float4& destination) {
	for (int32_t i = begin; i < end; ++i) {
		float4 const& color = source.at(i);

		destination.at(i) = float4(tonemap_function(color[0]),
								   tonemap_function(color[1]),
								   tonemap_function(color[2]),
								   color[3]);
	}
}

float Generic::tonemap_function(float x) const {
	x = std::min(x, hdr_max_);
	float z = std::pow(x, a_);
	return z / (std::pow(z, d_) * b_ + c_);
}

}}}
