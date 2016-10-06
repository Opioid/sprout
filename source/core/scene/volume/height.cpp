#include "height.hpp"
#include "base/json/json.hpp"
#include "base/math/vector.inl"

namespace scene { namespace volume {

Height::Height() : a_(1.f), b_(1.f) {}

float Height::density(float3_p p) const {
	return a_ * std::exp(-b_ * p.y);
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
