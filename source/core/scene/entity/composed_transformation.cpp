#include "composed_transformation.hpp"
#include "base/math/matrix4x4.inl"
#include "base/math/quaternion.inl"

namespace scene { namespace entity {

void Composed_transformation::set(const math::Transformation& t) {
	rotation = math::quaternion::create_matrix3x3(t.rotation);
	position = t.position;
	scale = t.scale;
	math::set_basis_scale_origin(object_to_world, rotation, scale, position);
	world_to_object = math::affine_inverted(object_to_world);
}

}}
