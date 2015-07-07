#include "material.hpp"
#include "image/texture/texture.inl"
#include "image/texture/texture_2d.inl"
#include "image/texture/sampler/sampler_2d.hpp"

namespace scene { namespace material {

IMaterial::IMaterial(std::shared_ptr<image::Image> mask) : mask_(mask) {}

math::float2 IMaterial::emission_importance_sample(math::float2 /*r2*/, float& /*pdf*/) const {
	return math::float2::identity;
}

float IMaterial::emission_pdf(math::float2 /*uv*/, const image::sampler::Sampler_2D& /*sampler*/) const {
	return 0.f;
}

void IMaterial::prepare_sampling() {}

bool IMaterial::is_masked() const {
	return mask_.is_valid();
}

float IMaterial::opacity(math::float2 uv, const image::sampler::Sampler_2D& sampler) const {
	if (mask_) {
		return sampler.sample1(mask_, uv);
	} else {
		return 1.f;
	}
}

}}
