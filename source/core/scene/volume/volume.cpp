#include "volume.hpp"

namespace scene { namespace volume {

void Volume::set_scene_aabb(const math::aabb& aabb) {
	atmosphere_y_ = aabb.max().y;
}

void Volume::on_set_transformation() {}

}}
