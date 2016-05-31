#include "material.hpp"
#include "image/texture/sampler/sampler_2d.hpp"
#include "scene/scene_worker.hpp"
#include "base/math/vector.inl"

namespace scene { namespace material {

Material::Material(std::shared_ptr<image::texture::Texture_2D> mask,
				   const Sampler_settings& sampler_settings, bool two_sided) :
	mask_(mask),
	sampler_key_(static_cast<uint32_t>(sampler_settings.filter)),
	two_sided_(two_sided) {}

void Material::tick(float /*absolute_time*/, float /*time_slice*/) {}

math::float3 Material::sample_radiance(math::pfloat3 /*wi*/, math::float2 /*uv*/,
									   float /*area*/, float /*time*/,
									   const Worker& /*worker*/, Sampler_filter /*filter*/) const {
	return math::float3(0.f, 0.f, 0.f);
}

math::float3 Material::average_radiance(float area) const {
	return math::float3(0.f, 0.f, 0.f);
}

bool Material::has_emission_map() const {
	return false;
}

math::float2 Material::radiance_importance_sample(math::float2 /*r2*/, float& /*pdf*/) const {
	return math::float2::identity;
}

float Material::emission_pdf(math::float2 /*uv*/, const Worker& /*worker*/,
							 Sampler_filter /*filter*/) const {
	return 0.f;
}

float Material::opacity(math::float2 uv, float /*time*/,
						const Worker& worker, Sampler_filter filter) const {
	if (mask_) {
		auto& sampler = worker.sampler(sampler_key_, filter);
		return sampler.sample_1(*mask_, uv);
	} else {
		return 1.f;
	}
}

void Material::prepare_sampling(bool /*spherical*/) {}

bool Material::is_animated() const {
	return false;
}

uint32_t Material::sampler_key() const {
	return sampler_key_;
}

bool Material::is_masked() const {
	return !mask_ == false;
}

bool Material::is_emissive() const {
	if (has_emission_map()) {
		return true;
	}

	math::float3 e = average_radiance(1.f);
	if (e.x > 0.f || e.y > 0.f || e.z > 0.f) {
		return true;
	}

	return false;
}

bool Material::is_two_sided() const {
	return two_sided_;
}

}}
