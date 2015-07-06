#include "triangle_mesh.hpp"
#include "triangle_primitive.inl"
#include "triangle_intersection.hpp"
#include "scene/entity/composed_transformation.hpp"
#include "scene/shape/shape_sample.hpp"
#include "scene/shape/geometry/shape_intersection.hpp"
#include "sampler/sampler.hpp"
#include "base/math/distribution_1d.inl"
#include "base/math/vector.inl"
#include "base/math/ray.inl"
#include "base/math/matrix.inl"

namespace scene { namespace shape { namespace triangle {

void Mesh::init() {
	aabb_ = tree_.aabb();

	distributions_.resize(tree_.num_parts());
}

uint32_t Mesh::num_parts() const {
	return tree_.num_parts();
}

bool Mesh::intersect(const entity::Composed_transformation& transformation, math::Oray& ray,
					 const math::float2& bounds, Node_stack& node_stack,
					 shape::Intersection& intersection) const {
	math::Oray tray;
	tray.origin = math::transform_point(transformation.world_to_object, ray.origin);
	tray.set_direction(math::transform_vector(transformation.world_to_object, ray.direction));
	tray.min_t = ray.min_t;
	tray.max_t = ray.max_t;

	Intersection pi;
	if (tree_.intersect(tray, bounds, node_stack, pi)) {
		intersection.epsilon = 3e-3f * tray.max_t;

		intersection.p = ray.point(tray.max_t);

		math::float3 n;
		math::float3 t;
		math::float2 uv;
		tree_.interpolate_triangle_data(pi.index, pi.uv, n, t, uv);

		intersection.geo_n = math::transform_vector(transformation.rotation, tree_.triangle_normal(pi.index));
	//	intersection.n = math::transform_vector(transformation.rotation, n);
	//	intersection.t = math::transform_vector(transformation.rotation, t);

		math::transform_vectors(transformation.rotation, n, t, intersection.n, intersection.t);

		intersection.b = math::cross(intersection.n, intersection.t);
		intersection.uv = uv;
		intersection.part = tree_.triangle_material_index(pi.index);

		ray.max_t = tray.max_t;
		return true;
	}

	return false;
}

bool Mesh::intersect_p(const entity::Composed_transformation& transformation, const math::Oray& ray,
					   const math::float2& bounds, Node_stack& node_stack) const {
	math::Oray tray;
	tray.origin = math::transform_point(transformation.world_to_object, ray.origin);
	tray.set_direction(math::transform_vector(transformation.world_to_object, ray.direction));
	tray.min_t = ray.min_t;
	tray.max_t = ray.max_t;

	return tree_.intersect_p(tray, bounds, node_stack);
}

float Mesh::opacity(const entity::Composed_transformation& transformation, const math::Oray& ray,
					const math::float2& bounds, Node_stack& node_stack,
					const material::Materials& materials, const image::sampler::Sampler_2D& sampler) const {
	math::Oray tray;
	tray.origin = math::transform_point(transformation.world_to_object, ray.origin);
	tray.set_direction(math::transform_vector(transformation.world_to_object, ray.direction));
	tray.min_t = ray.min_t;
	tray.max_t = ray.max_t;

	return tree_.opacity(tray, bounds, node_stack, materials, sampler);
}

void Mesh::sample(uint32_t part, const entity::Composed_transformation& transformation, float area,
				  const math::float3& p, const math::float3& n,
				  sampler::Sampler& sampler, Sample& sample) const {
	float r = sampler.generate_sample_1D();
	math::float2 r2 = sampler.generate_sample_2D();

	uint32_t index = distributions_[part].distribution.sample_discrete(r);

	math::float3 sv;
	math::float3 sn;
	math::float2 tc;
	tree_.sample(index, r2, sv, sn, tc);

	math::float3 v = math::transform_point(transformation.object_to_world, sv);
	math::float3 wn = math::transform_vector(transformation.rotation, sn);

	math::float3 axis = v - p;

	sample.wi = math::normalized(axis);

	float c = math::dot(wn, -sample.wi);

	if (c <= 0.f) {
		sample.pdf = 0.f;
	} else {
		float sl = math::squared_length(axis);
		sample.t = std::sqrt(sl);
		sample.pdf = sl / (c * area);
	}
}

void Mesh::sample(uint32_t part, const entity::Composed_transformation& transformation, float area,
				  const math::float3& p, const math::float2& uv, Sample& sample) const {}

float Mesh::pdf(uint32_t /*part*/, const entity::Composed_transformation& /*transformation*/, float /*area*/,
				const math::float3& /*p*/, const math::float3& /*wi*/) const {
	return 1.f;
}

float Mesh::area(uint32_t part, const math::float3& /*scale*/) const {
	return distributions_[part].distribution.integral();
}

bool Mesh::is_complex() const {
	return true;
}

void Mesh::prepare_sampling(uint32_t part, const math::float3& scale) {
	distributions_[part].init(part, tree_.triangles(), scale);
}

void Mesh::Distribution::init(uint32_t part, const std::vector<Triangle>& triangles, const math::float3& scale) {
	std::vector<float> areas;

	triangle_mapping.clear();

	uint32_t i = 0;
	for (auto& t : triangles) {
		if (t.material_index == part) {
			areas.push_back(t.area(scale));
			triangle_mapping.push_back(i);
		}

		++i;
	}

	distribution.init(areas.data(), areas.size());
}

uint32_t Mesh::Distribution::sample(float r) {
	return triangle_mapping[distribution.sample_discrete(r)];
}

}}}


