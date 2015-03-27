#include "triangle_mesh.hpp"
#include "triangle_primitive.inl"
#include "triangle_intersection.hpp"
#include "scene/entity/composed_transformation.hpp"
#include "scene/shape/geometry/shape_intersection.hpp"
#include "base/math/vector.inl"
#include "base/math/ray.inl"
#include "base/math/matrix.inl"

namespace scene { namespace shape { namespace triangle {

bool Mesh::intersect(const Composed_transformation& transformation, const math::Oray& ray, const math::float2& bounds,
					 shape::Intersection& intersection, float& hit_t) const {
	math::Oray tray;
	tray.origin = math::transform_point(transformation.world_to_object, ray.origin);
	tray.set_direction(math::transform_vector(transformation.world_to_object, ray.direction));
	tray.min_t = ray.min_t;
	tray.max_t = ray.max_t;

	Intersection pi;
	if (tree_.intersect(tray, bounds, pi)) {
		intersection.epsilon = 3e-3f * pi.c.t;

		intersection.p = ray.point(pi.c.t);

		math::float3 n;
		math::float3 t;
		math::float2 uv;
		tree_.triangles_[pi.index].interpolate(pi.c.u, pi.c.v, n, t, uv);

		intersection.n = math::transform_vector(transformation.rotation, n);
		intersection.t = math::transform_vector(transformation.rotation, t);
		intersection.b = math::cross(intersection.n, intersection.t);
		intersection.uv = uv;
		intersection.material_index = tree_.triangles_[pi.index].material_index;

		hit_t = pi.c.t;
		return true;
	}

	return false;
}

bool Mesh::intersect_p(const Composed_transformation& transformation, const math::Oray& ray, const math::float2& bounds) const {
	math::Oray tray;
	tray.origin = math::transform_point(transformation.world_to_object, ray.origin);
	tray.set_direction(math::transform_vector(transformation.world_to_object, ray.direction));
	tray.min_t = ray.min_t;
	tray.max_t = ray.max_t;

	return tree_.intersect_p(tray, bounds);
}

void Mesh::importance_sample(const Composed_transformation& transformation, const math::float3& p, const math::float2& sample,
							 math::float3& wi, float& t, float& pdf) const {}

bool Mesh::is_complex() const {
	return true;
}

}}}


