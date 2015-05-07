#include "material.hpp"
#include "image/texture/sampler/sampler_2d.hpp"

namespace scene { namespace material {

IMaterial::IMaterial(std::shared_ptr<image::Image> mask) : mask_(mask) {}

float IMaterial::opacity(math::float2 uv, const image::sampler::Sampler_2D& sampler) const {
	if (mask_) {
		return sampler.sample3(mask_, uv).x;
	} else {
		return 1.f;
	}
}

}}
