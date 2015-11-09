#include "shape.hpp"

namespace scene { namespace shape {

Shape::~Shape() {}

const math::aabb& Shape::aabb() const {
	return aabb_;
}

uint32_t Shape::num_parts() const {
	return 1;
}

bool Shape::is_complex() const {
	return false;
}

bool Shape::is_finite() const {
	return true;
}

bool Shape::is_analytical() const {
	return true;
}

void Shape::prepare_sampling(uint32_t /*part*/) {}

Morphable_shape* Shape::morphable_shape() {
	return nullptr;
}

}}
