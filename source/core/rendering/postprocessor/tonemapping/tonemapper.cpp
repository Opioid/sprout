#include "tonemapper.hpp"
#include "image/typed_image.inl"
#include "base/math/vector3.inl"

namespace rendering { namespace postprocessor { namespace tonemapping {

Tonemapper::~Tonemapper() {}

void Tonemapper::init(const scene::camera::Camera& /*camera*/, thread::Pool& /*pool*/) {}

size_t Tonemapper::num_bytes() const {
	return 0;
}

void Tonemapper::apply(int32_t begin, int32_t end, uint32_t /*pass*/,
					   const image::Float_4& source,
					   image::Float_4& destination) {
	for (int32_t i = begin; i < end; ++i) {
		const float4& color = source.at(i);

		float3 tonemapped = tonemap(color.xyz());

		destination.at(i) = float4(tonemapped, color[3]);
	}
}

float Tonemapper::normalization_factor(float linear_max, float tonemapped_max) {
	return linear_max > 0.f ? 1.f / tonemapped_max : 1.f;
}

}}}
