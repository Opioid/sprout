#include "postprocessor_backplate.hpp"
#include "image/typed_image.inl"
#include "image/texture/texture.hpp"
#include "base/math/vector4.inl"

namespace rendering::postprocessor {

Backplate::Backplate(const Texture_ptr& backplate) :
	Postprocessor(1),
	backplate_(backplate) {}

void Backplate::init(const scene::camera::Camera& /*camera*/, thread::Pool& /*pool*/) {}

bool Backplate::alpha_out(bool /*alpha_in*/) const {
	return false;
}

size_t Backplate::num_bytes() const {
	return sizeof(*this);
}

void Backplate::apply(uint32_t /*id*/, uint32_t /*pass*/, int32_t begin, int32_t end,
					  const image::Float4& source, image::Float4& destination) {
	for (int32_t i = begin; i < end; ++i) {
		const float4 foreground = source.load(i);

		const float3 background = backplate_->at_3(i);

		const float alpha = foreground[3];

		destination.store(i, float4(foreground.xyz() + (1.f - alpha) * background, 1.f));
	}
}

}