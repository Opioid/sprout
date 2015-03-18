#include "shape.hpp"

namespace scene { namespace shape {

const math::AABB& Shape::aabb() const {
	return aabb_;
}

bool Shape::is_complex() const {
	return false;
}

bool Shape::is_finite() const {
	return true;
}

}}
