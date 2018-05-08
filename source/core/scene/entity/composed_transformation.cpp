#include "composed_transformation.hpp"
#include "base/math/matrix4x4.inl"
#include "base/math/quaternion.inl"
#include "base/math/transformation.inl"

namespace scene::entity {

void Composed_transformation::set(math::Transformation const& t) {
	rotation = math::quaternion::create_matrix3x3(t.rotation);
	position = t.position;
	scale = t.scale;

	object_to_world = math::compose(rotation, scale, position);
	world_to_object = math::affine_inverted(object_to_world);
}

float3 Composed_transformation::world_to_object_point(f_float3 p) const {
	return math::transform_point(p, world_to_object);
}

float3 Composed_transformation::world_to_object_vector(f_float3 v) const {
	return math::transform_vector(v, world_to_object);
}

}
