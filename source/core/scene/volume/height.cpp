#include "height.hpp"
#include "base/json/json.hpp"
#include "base/math/vector.inl"
#include "base/math/bounding/aabb.inl"

namespace scene { namespace volume {

Height::Height() : a_(1.f), b_(1.f) {}

float Height::density(float3_p p, Worker& /*worker*/, Sampler_filter /*filter*/) const {
	// p is in object space already

	if (!local_aabb_.intersect(p)) {
		return 0.f;
	}

	float height = 0.5f * (1.f + p.y);

	return a_ * std::exp(-b_ * height);
}

void Height::set_parameter(const std::string& name,
						   const json::Value& value) {
	if ("a" == name) {
		a_ = json::read_float(value);
	} else if ("b" == name) {
		b_ = json::read_float(value);
	}
}

}}
