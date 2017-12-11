#include "volume.hpp"
#include "base/json/json.hpp"
#include "base/math/aabb.inl"
#include "base/math/ray.inl"
#include "base/math/vector3.inl"

namespace scene::volume {

Volume::Volume() : match_scene_scale_(false) {}

void Volume::set_parameters(const json::Value& parameters) {
	for (auto& n : parameters.GetObject()) {
		if ("match_scene_scale" == n.name) {
			match_scene_scale_ = json::read_bool(n.value);
		}
	}
}

void Volume::set_parameter(const std::string& /*name*/, const json::Value& /*value*/) {}

void Volume::set_scene_aabb(const math::AABB& aabb) {
	if (match_scene_scale_) {
		set_transformation(math::Transformation{
			aabb.position(),
			aabb.halfsize(),
			math::quaternion::create(world_transformation_.rotation)
		});
	}
}

}
