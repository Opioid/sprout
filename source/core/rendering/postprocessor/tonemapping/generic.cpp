#include "generic.hpp"
#include "image/typed_image.inl"
#include "base/math/vector4.inl"

namespace rendering { namespace postprocessor { namespace tonemapping {

Generic::Generic(float contrast, float shoulder, float mid_in, float mid_out, float hdr_max) :
	a_(contrast),
	d_(shoulder),
	hdr_max_(hdr_max) {
	const float ad = contrast * shoulder;

	const float midi_pow_a  = std::pow(mid_in, contrast);
	const float midi_pow_ad = std::pow(mid_in, ad);
	const float hdrm_pow_a  = std::pow(hdr_max, contrast);
	const float hdrm_pow_ad = std::pow(hdr_max, ad);

	const float u = hdrm_pow_ad * mid_out - midi_pow_ad * mid_out;
	const float v = midi_pow_ad * mid_out;

	b_ = -((-midi_pow_a + (mid_out * (hdrm_pow_ad * midi_pow_a - hdrm_pow_a * v)) / u) / v);
	c_ = (hdrm_pow_ad * midi_pow_a - hdrm_pow_a * v) / u;
}

void Generic::apply(int32_t begin, int32_t end, uint32_t /*pass*/,
					const image::Float4& source, image::Float4& destination) {
	for (int32_t i = begin; i < end; ++i) {
		const float4& color = source.at(i);

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
