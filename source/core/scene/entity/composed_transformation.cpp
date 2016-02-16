#include "composed_transformation.hpp"
#include "base/math/matrix.inl"
#include "base/math/quaternion.inl"

namespace scene { namespace entity {

void Composed_transformation::set(const math::transformation& t) {
	position = t.position;
	scale = t.scale;
	rotation = math::create_matrix4x4(t.rotation);
	math::set_basis_scale_origin(object_to_world, rotation, t.scale, t.position);
	world_to_object = math::affine_inverted(object_to_world);
}

}}
