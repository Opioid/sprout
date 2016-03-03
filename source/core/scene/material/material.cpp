#include "material.hpp"
#include "image/texture/sampler/sampler_2d.hpp"
#include "base/math/vector.inl"

namespace scene { namespace material {

Material::Material(std::shared_ptr<image::texture::Texture_2D> mask, bool two_sided) :
	mask_(mask), two_sided_(two_sided) {}

void Material::tick(float /*absolute_time*/, float /*time_slice*/) {}

math::float2 Material::emission_importance_sample(math::float2 /*r2*/, float& /*pdf*/) const {
	return math::float2::identity;
}

float Material::emission_pdf(math::float2 /*uv*/, const image::texture::sampler::Sampler_2D& /*sampler*/) const {
	return 0.f;
}

float Material::opacity(math::float2 uv, float /*time*/, const image::texture::sampler::Sampler_2D& sampler) const {
	if (mask_) {
		return sampler.sample_1(*mask_, uv);
	} else {
		return 1.f;
	}
}

void Material::prepare_sampling(bool /*spherical*/) {}

bool Material::is_animated() const {
	return false;
}

bool Material::is_masked() const {
	return !mask_ == false;
}

bool Material::is_emissive() const {
	if (has_emission_map()) {
		return true;
	}

	math::float3 e = average_emission();
	if (e.x > 0.f || e.y > 0.f || e.z > 0.f) {
		return true;
	}

	return false;
}

bool Material::is_two_sided() const {
	return two_sided_;
}

}}
