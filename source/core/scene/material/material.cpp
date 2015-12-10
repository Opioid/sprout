#include "material.hpp"
#include "image/texture/sampler/sampler_2d.hpp"
#include "base/math/vector.inl"

namespace scene { namespace material {

IMaterial::IMaterial(std::shared_ptr<image::texture::Texture_2D> mask, bool two_sided) :
	mask_(mask), two_sided_(two_sided) {}

math::float2 IMaterial::emission_importance_sample(math::float2 /*r2*/, float& /*pdf*/) const {
	return math::float2::identity;
}

float IMaterial::emission_pdf(math::float2 /*uv*/,
							  const image::texture::sampler::Sampler_2D& /*sampler*/) const {
	return 0.f;
}

void IMaterial::prepare_sampling(bool /*spherical*/) {}

bool IMaterial::is_masked() const {
	return !mask_ == false;
}

bool IMaterial::is_emissive() const {
	if (emission_map()) {
		return true;
	}

	math::float3 e = average_emission();
	if (e.x > 0.f || e.y > 0.f || e.z > 0.f) {
		return true;
	}

	return false;
}

bool IMaterial::is_two_sided() const {
	return two_sided_;
}

float IMaterial::opacity(math::float2 uv, const image::texture::sampler::Sampler_2D& sampler) const {
	if (mask_) {
		return sampler.sample_1(*mask_, uv);
	} else {
		return 1.f;
	}
}

}}
