#include "composed_transformation.hpp"
#include "base/math/matrix.inl"
#include "base/math/quaternion.inl"

namespace scene {

void Composed_transformation::set(const math::float3& p, const math::float3& s, const math::quaternion& r) {
	position = p;
	scale = s;
	rotation = math::float3x3(r);
	math::set_basis_scale_origin(object_to_world, rotation, s, p);
	world_to_object = math::inverted(object_to_world);
}

}
