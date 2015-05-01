#include "triangle_mesh.hpp"
#include "triangle_primitive.inl"
#include "triangle_intersection.hpp"
#include "scene/entity/composed_transformation.hpp"
#include "scene/shape/geometry/shape_intersection.hpp"
#include "sampler/sampler.hpp"
#include "base/math/vector.inl"
#include "base/math/ray.inl"
#include "base/math/matrix.inl"

namespace scene { namespace shape { namespace triangle {

bool Mesh::intersect(const Composed_transformation& transformation, const math::Oray& ray,
					 const math::float2& bounds, Node_stack& node_stack,
					 shape::Intersection& intersection, float& hit_t) const {
	math::Oray tray;
	tray.origin = math::transform_point(transformation.world_to_object, ray.origin);
	tray.set_direction(math::transform_vector(transformation.world_to_object, ray.direction));
	tray.min_t = ray.min_t;
	tray.max_t = ray.max_t;

	Intersection pi;
	if (tree_.intersect(tray, bounds, node_stack, pi)) {
		intersection.epsilon = 3e-3f * pi.c.t;

		intersection.p = ray.point(pi.c.t);

		math::float3 n;
		math::float3 t;
		math::float2 uv;
		tree_.interpolate_triangle_data(pi.index, pi.c.uv, n, t, uv);

		intersection.n = math::transform_vector(transformation.rotation, n);
		intersection.t = math::transform_vector(transformation.rotation, t);
		intersection.b = math::cross(intersection.n, intersection.t);
		intersection.uv = uv;
		intersection.material_index = tree_.triangle_material_index(pi.index);

		hit_t = pi.c.t;
		return true;
	}

	return false;
}

bool Mesh::intersect_p(const Composed_transformation& transformation, const math::Oray& ray,
					   const math::float2& bounds, Node_stack& node_stack) const {
	math::Oray tray;
	tray.origin = math::transform_point(transformation.world_to_object, ray.origin);
	tray.set_direction(math::transform_vector(transformation.world_to_object, ray.direction));
	tray.min_t = ray.min_t;
	tray.max_t = ray.max_t;

	return tree_.intersect_p(tray, bounds, node_stack);
}

void Mesh::importance_sample(const Composed_transformation& transformation, const math::float3& p,
							 sampler::Sampler& sampler, uint32_t sample_index,
							 math::float3& wi, float& t, float& pdf) const {
	float r = sampler.generate_sample1d(sample_index);
	math::float2 sample = sampler.generate_sample2d(sample_index);

	math::float3 sv;
	math::float3 sn;
	math::float2 tc;
	tree_.importance_sample(r, sample, sv, sn, tc);

	math::float3 v = math::transform_point(transformation.object_to_world, sv);
	math::float3 n = math::transform_vector(transformation.rotation, sn);

	math::float3 axis = v - p;

	wi = math::normalized(axis);

	float c = math::dot(n, -wi);

	if (c <= 0.f) {
		pdf = 0.f;
	} else {
		float sl = math::squared_length(axis);
		t = std::sqrt(sl);
		pdf = sl / (c * area(transformation.scale));
	}
}

float Mesh::area(const math::float3& /*scale*/) const {
	return 0.25f;
}

bool Mesh::is_complex() const {
	return true;
}

}}}


