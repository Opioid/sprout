#include "volume.hpp"
#include "base/math/vector.inl"

namespace scene { namespace volume {

void Volume::set_parameters(const json::Value& /*parameters*/) {}

void Volume::set_scene_aabb(const math::aabb& aabb) {
	scene_bb_ = aabb;
}

void Volume::on_set_transformation() {}

}}
