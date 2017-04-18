#include "aces.hpp"
#include "image/typed_image.inl"
#include "base/math/vector4.inl"

namespace rendering { namespace postprocessor { namespace tonemapping {

Aces::Aces(float hdr_max) :
	normalization_factor_(normalization_factor(hdr_max, tonemap_function(hdr_max))) {}

void Aces::apply(int32_t begin, int32_t end, uint32_t /*pass*/,
				 const image::Float_4& source, image::Float_4& destination) {
	float norm = normalization_factor_;
	for (int32_t i = begin; i < end; ++i) {
		const float4& color = source.at(i);

		destination.at(i) = float4(norm * tonemap_function(color[0]),
								   norm * tonemap_function(color[1]),
								   norm * tonemap_function(color[2]),
								   color[3]);
	}
}

float Aces::tonemap_function(float x) {
	// ACES like in https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/
	float a = 2.51f;
	float b = 0.03f;
	float c = 2.43f;
	float d = 0.59f;
	float e = 0.14f;

	return (x * (a * x + b)) / (x * (c * x + d) + e);
}

}}}
