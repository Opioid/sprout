#include "material.hpp"
#include "bssrdf.hpp"
#include "material_sample_cache.inl"
#include "image/texture/texture_adapter.inl"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.hpp"
#include "base/json/json.hpp"
#include "base/math/vector4.inl"

namespace scene { namespace material {

Material::Material(Sample_cache& sample_cache, const Sampler_settings& sampler_settings,
				   bool two_sided) :
	sample_cache_(sample_cache),
	sampler_key_(sampler_settings.key()),
	two_sided_(two_sided) {}

Material::~Material() {}

void Material::set_mask(const Texture_adapter& mask) {
	mask_ = mask;
}

void Material::set_parameters(const json::Value& parameters) {
	for (auto& n : parameters.GetObject()) {
		set_parameter(n.name.GetString(), n.value);
	}
}

void Material::tick(float /*absolute_time*/, float /*time_slice*/) {}

const BSSRDF& Material::bssrdf(const Worker& worker) {
	return sample_cache_.bssrdf(worker.id());
}

float3 Material::sample_radiance(const float3& /*wi*/, float2 /*uv*/, float /*area*/,
								 float /*time*/, const Worker& /*worker*/,
								 Sampler_filter /*filter*/) const {
	return float3(0.f);
}

float3 Material::average_radiance(float /*area*/) const {
	return float3(0.f);
}

bool Material::has_emission_map() const {
	return false;
}

float2 Material::radiance_sample(float2 r2, float& pdf) const {
	pdf = 1.f;
	return r2;
}

float Material::emission_pdf(float2 /*uv*/, const Worker& /*worker*/,
							 Sampler_filter /*filter*/) const {
	return 1.f;
}

float Material::opacity(float2 uv, float /*time*/, const Worker& worker,
						Sampler_filter filter) const {
	if (mask_.is_valid()) {
		auto& sampler = worker.sampler_2D(sampler_key_, filter);
		return mask_.sample_1(sampler, uv);
	} else {
		return 1.f;
	}
}

float3 Material::thin_absorption(const float3& /*wo*/, const float3& /*n*/, float2 uv, float time,
								 const Worker& worker, Sampler_filter filter) const {
	return float3(opacity(uv, time, worker, filter));
}

void Material::prepare_sampling(const shape::Shape& /*shape*/, uint32_t /*part*/,
								const Transformation& /*transformation*/, float /*area*/,
								bool /*importance_sampling*/, thread::Pool& /*pool*/) {}

bool Material::is_animated() const {
	return false;
}

bool Material::is_subsurface() const {
	return false;
}

bool Material::is_translucent() const {
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

	const float3 e = average_radiance(1.f);
	return math::any_greater_zero(e);
}

bool Material::is_two_sided() const {
	return two_sided_;
}

void Material::set_parameter(const std::string& /*name*/, const json::Value& /*value*/) {}

}}
