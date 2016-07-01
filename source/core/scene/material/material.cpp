#include "material.hpp"
#include "image/texture/texture_2d_adapter.inl"
#include "scene/scene_worker.hpp"
#include "base/json/json.hpp"
#include "base/math/vector.inl"

namespace scene { namespace material {

Material::Material(const Sampler_settings& sampler_settings, bool two_sided) :
	sampler_key_(static_cast<uint32_t>(sampler_settings.filter)),
	two_sided_(two_sided) {}

Material::~Material() {}

void Material::set_mask(const Adapter_2D& mask) {
	mask_ = mask;
}

void Material::set_parameters(const json::Value& parameters) {
	for (auto n = parameters.MemberBegin(); n != parameters.MemberEnd(); ++n) {
		const std::string node_name = n->name.GetString();
		const json::Value& node_value = n->value;

		set_parameter(node_name, node_value);
	}
}

void Material::tick(float /*absolute_time*/, float /*time_slice*/) {}

float3 Material::sample_radiance(float3_p /*wi*/, float2 /*uv*/, float /*area*/, float /*time*/,
								 const Worker& /*worker*/, Sampler_filter /*filter*/) const {
	return float3(0.f, 0.f, 0.f);
}

float3 Material::average_radiance(float /*area*/) const {
	return float3(0.f, 0.f, 0.f);
}

bool Material::has_emission_map() const {
	return false;
}

float2 Material::radiance_importance_sample(float2 /*r2*/, float& /*pdf*/) const {
	return float2::identity;
}

float Material::emission_pdf(float2 /*uv*/, const Worker& /*worker*/,
							 Sampler_filter /*filter*/) const {
	return 0.f;
}

float Material::opacity(float2 uv, float /*time*/, const Worker& worker,
						Sampler_filter filter) const {
	if (mask_.is_valid()) {
		auto& sampler = worker.sampler(sampler_key_, filter);
		return mask_.sample_1(sampler, uv);
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
	return mask_.is_valid();
}

bool Material::is_emissive() const {
	if (has_emission_map()) {
		return true;
	}

	float3 e = average_radiance(1.f);
	if (e.x > 0.f || e.y > 0.f || e.z > 0.f) {
		return true;
	}

	return false;
}

bool Material::is_two_sided() const {
	return two_sided_;
}

void Material::set_parameter(const std::string& name,
							 const json::Value& value) {

}

}}
