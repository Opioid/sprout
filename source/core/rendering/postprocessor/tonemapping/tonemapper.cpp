#include "tonemapper.hpp"
#include "image/typed_image.inl"
#include "base/math/vector.inl"

namespace rendering { namespace postprocessor { namespace tonemapping {

Tonemapper::~Tonemapper() {}

void Tonemapper::init(const scene::camera::Camera& /*camera*/) {}

void Tonemapper::apply(int32_t begin, int32_t end, uint32_t /*pass*/,
					   const image::Image_float_4& source,
					   image::Image_float_4& destination) {
	for (int32_t i = begin; i < end; ++i) {
		const float4& color = source.at(i);

		float3 tonemapped = tonemap(color.xyz);

		destination.at(i) = float4(tonemapped, color.w);
	}
}

float Tonemapper::normalization_factor(float linear_max, float tonemapped_max) {
	return linear_max > 0.f ? 1.f / tonemapped_max : 1.f;
}

}}}
