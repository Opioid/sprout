#include "volume.hpp"
#include "base/json/json.hpp"
#include "base/math/aabb.inl"
#include "base/math/vector3.inl"

namespace scene::volume {

Volume::Volume() :
	local_aabb_(float3(-1.f), float3(1.f)),
	absorption_(0.f), scattering_(0.f), anisotropy_(0.f),
	match_scene_scale_(false) {}

float Volume::phase(const float3& w, const float3& wp) const {
	const float g = anisotropy_;
	const float k = 1.55f * g - (0.55f * g) * (g * g);
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
		} else if ("match_scene_scale" == n.name) {
			match_scene_scale_ = json::read_bool(n.value);
		} else {
			set_parameter(n.name.GetString(), n.value);
		}
	}
}

void Volume::set_scene_aabb(const math::AABB& aabb) {
	scene_bb_ = aabb;

	if (match_scene_scale_) {
		set_transformation(math::Transformation{
			aabb.position(),
			aabb.halfsize(),
			math::quaternion::create(world_transformation_.rotation)
		});
	}
}

const math::AABB& Volume::aabb() const {
	return aabb_;
}

void Volume::on_set_transformation() {
	aabb_ = local_aabb_.transform(world_transformation_.object_to_world);
}

float Volume::phase_schlick(const float3& w, const float3& wp, float k) {
	const float d = 1.f - (k * math::dot(w, wp));
	return 1.f / (4.f * math::Pi) * (1.f - k * k) / (d * d);
}

}
