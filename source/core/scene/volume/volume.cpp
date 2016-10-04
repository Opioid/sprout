#include "volume.hpp"
#include "base/json/json.hpp"
#include "base/math/vector.inl"
#include "base/math/bounding/aabb.inl"

namespace scene { namespace volume {

Volume::Volume() : absorption_(0.f), scattering_(0.f), anisotropy_(0.f) {}

void Volume::set_parameters(const json::Value& parameters) {
	for (auto& n : parameters.GetObject()) {
		if ("absorption" == n.name) {
			absorption_ = json::read_float3(n.value);
		} else if ("scattering" == n.name) {
			scattering_ = json::read_float3(n.value);
		} else if ("anisotropy" == n.name) {
			anisotropy_ = json::read_float(n.value);
		}
	}
}

void Volume::set_scene_aabb(const math::aabb& aabb) {
	scene_bb_ = aabb;
}

void Volume::on_set_transformation() {}

}}
