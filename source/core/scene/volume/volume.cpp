#include "volume.hpp"

namespace scene { namespace volume {

void Volume::set_scene_aabb(const math::aabb& aabb) {
	scene_bb_ = aabb;
}

void Volume::on_set_transformation() {}

}}
