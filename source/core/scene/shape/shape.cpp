#include "shape.hpp"
#include "base/math/aabb.inl"
#include "base/math/vector.inl"
#include "base/math/matrix.inl"

namespace scene { namespace shape {

Shape::~Shape() {}

const math::aabb& Shape::aabb() const {
	return aabb_;
}

math::aabb Shape::transformed_aabb(const float4x4& m, const math::transformation& /*t*/) const {
	return aabb_.transform(m);
}

math::aabb Shape::transformed_aabb(const math::transformation& t) const {
	return transformed_aabb(float4x4(t), t);
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
