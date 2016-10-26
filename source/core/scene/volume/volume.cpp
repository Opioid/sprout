#include "volume.hpp"
#include "base/json/json.hpp"
#include "base/math/vector.inl"
#include "base/math/bounding/aabb.inl"

namespace scene { namespace volume {

Volume::Volume() :
	local_aabb_(float3(-1.f), float3(1.f)),
	absorption_(0.f), scattering_(0.f), anisotropy_(0.f) {}

float Volume::phase(float3_p w, float3_p wp) const {
	float g = anisotropy_;
	float k = 1.55f * g - 0.55f * g * g * g;
	return phase_schlick(w, wp, k);
}

void Volume::set_parameters(const json::Value& parameters) {
	for (auto& n : parameters.GetObject()) {
		if ("absorption" == n.name) {
			absorption_ = json::read_float3(n.value);
		} else if ("scattering" == n.name) {
			scattering_ = json::read_float3(n.value);
		} else if ("anisotropy" == n.name) {
			anisotropy_ = json::read_float(n.value);
		} else {
			set_parameter(n.name.GetString(), n.value);
		}
	}
}

void Volume::set_scene_aabb(const math::aabb& aabb) {
	scene_bb_ = aabb;
}

const math::aabb& Volume::aabb() const {
	return aabb_;
}

void Volume::on_set_transformation() {
	aabb_ = local_aabb_.transform(world_transformation_.object_to_world);
}

float Volume::phase_schlick(float3_p w, float3_p wp, float k) {
	float d = 1.f - (k * math::dot(w, wp));
	return 1.f / (4.f * math::Pi) * (1.f - k * k) / (d * d);
}

}}
