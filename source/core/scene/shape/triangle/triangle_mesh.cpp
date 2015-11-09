#include "triangle_mesh.hpp"
#include "triangle_intersection.hpp"
#include "bvh/triangle_bvh_tree.inl"
#include "bvh/triangle_bvh_data_mt.inl"
#include "scene/entity/composed_transformation.hpp"
#include "scene/shape/shape_sample.hpp"
#include "scene/shape/geometry/shape_intersection.hpp"
#include "sampler/sampler.hpp"
#include "base/math/vector.inl"
#include "base/math/ray.inl"
#include "base/math/matrix.inl"
#include "base/math/distribution/distribution_1d.inl"

#include <iostream>
#include "base/math/print.hpp"

namespace scene { namespace shape { namespace triangle {

void Mesh::init() {
	aabb_ = tree_.aabb();

	distributions_.resize(tree_.num_parts());
}

uint32_t Mesh::num_parts() const {
	return tree_.num_parts();
}

bool Mesh::intersect(const entity::Composed_transformation& transformation, math::Oray& ray,
					 Node_stack& node_stack, shape::Intersection& intersection) const {
	math::Oray tray;
	tray.origin = math::transform_point(transformation.world_to_object, ray.origin);
	tray.set_direction(math::transform_vector(transformation.world_to_object, ray.direction));
	tray.min_t = ray.min_t;
	tray.max_t = ray.max_t;

	Intersection pi;
	if (tree_.intersect(tray, node_stack, pi)) {
		intersection.epsilon = 3e-3f * tray.max_t;

		intersection.p = ray.point(tray.max_t);

	//	math::float3 a = ray.point(tray.max_t);

	//	math::float3 sv;
	//	math::float2 tc;
	//	tree_.sample(pi.index, pi.uv, sv, tc);
	//	intersection.p = math::transform_point(transformation.object_to_world, tray.point(tray.max_t));

	//	if (std::abs(math::distance(a, intersection.p)) > 0.0001f) {
	//	if (std::abs(tray.max_t - math::distance(intersection.p, ray.origin)) > 0.0001f) {
	//		std::cout << "alarm" << std::endl;
	//	}

		math::float3 n;
		math::float3 t;
		math::float2 uv;
		tree_.interpolate_triangle_data(pi.index, pi.uv, n, t, uv);

		intersection.geo_n = math::transform_vector(transformation.rotation, tree_.triangle_normal(pi.index));
	//	intersection.n = math::transform_vector(transformation.rotation, n);
	//	intersection.t = math::transform_vector(transformation.rotation, t);

		math::transform_vectors(transformation.rotation, n, t, intersection.n, intersection.t);

		float bitangent_sign = tree_.triangle_bitangent_sign(pi.index);
		intersection.b = bitangent_sign * math::cross(intersection.n, intersection.t);
		intersection.uv = uv;
		intersection.part = tree_.triangle_material_index(pi.index);

		ray.max_t = tray.max_t;
		return true;
	}

	return false;
}

bool Mesh::intersect_p(const entity::Composed_transformation& transformation, const math::Oray& ray,
					   Node_stack& node_stack) const {
	math::Oray tray;
	tray.origin = math::transform_point(transformation.world_to_object, ray.origin);
	tray.set_direction(math::transform_vector(transformation.world_to_object, ray.direction));
	tray.min_t = ray.min_t;
	tray.max_t = ray.max_t;

	return tree_.intersect_p(tray, node_stack);
}

float Mesh::opacity(const entity::Composed_transformation& transformation, const math::Oray& ray,
					Node_stack& node_stack, const material::Materials& materials,
					const image::texture::sampler::Sampler_2D& sampler) const {
	math::Oray tray;
	tray.origin = math::transform_point(transformation.world_to_object, ray.origin);
	tray.set_direction(math::transform_vector(transformation.world_to_object, ray.direction));
	tray.min_t = ray.min_t;
	tray.max_t = ray.max_t;

	return tree_.opacity(tray, node_stack, materials, sampler);
}

void Mesh::sample(uint32_t part, const entity::Composed_transformation& transformation, float area,
				  const math::float3& p, const math::float3& /*n*/, bool /*total_sphere*/,
				  sampler::Sampler& sampler, Node_stack& node_stack, Sample& sample) const {
/*	float r = sampler.generate_sample_1D();
	math::float2 r2 = sampler.generate_sample_2D();

	uint32_t index = distributions_[part].distribution.sample_discrete(r);

	math::float3 sv;
	tree_.sample(index, r2, sv);
	math::float3 v = math::transform_point(transformation.object_to_world, sv);

	math::float3 axis = v - p;
	math::float3 dir = math::normalized(axis);

	math::Oray ray;
	ray.origin = math::transform_point(transformation.world_to_object, p);
	ray.set_direction(math::transform_vector(transformation.world_to_object, dir));
	ray.min_t = 0.f;
	ray.max_t = 10000.f;

	Intersection pi;
	if (tree_.intersect(ray, node_stack, pi)) {
		uint32_t shape_part = tree_.triangle_material_index(pi.index);
		if (part != shape_part) {
			sample.pdf = 0.f;
			return;
		}

		math::float3 sn = tree_.triangle_normal(pi.index);
		math::float3 wn = math::transform_vector(transformation.rotation, sn);

		float c = math::dot(wn, -dir);
		if (c <= 0.f) {
			sample.pdf = 0.f;
			return;
		}

		sample.wi = dir;
		sample.uv = tree_.interpolate_triangle_uv(pi.index, pi.uv);
		sample.t = ray.max_t;
		float sl = ray.max_t * ray.max_t;
		sample.pdf = sl / (c * area);

		// kubitza
		{
			math::float3 sn = tree_.triangle_normal(index);
			math::float3 wn = math::transform_vector(transformation.rotation, sn);

			float c = math::dot(wn, -dir);
			if (c <= 0.f) {
				sample.pdf = 0.f;
			}
		}

		return;
	} else {
		sample.pdf = 0.f;
	}
*/


	float r = sampler.generate_sample_1D();
	math::float2 r2 = sampler.generate_sample_2D();

	uint32_t index = distributions_[part].sample(r);

	math::float3 sv;
	math::float2 tc;
	tree_.sample(index, r2, sv, tc);
	math::float3 v = math::transform_point(transformation.object_to_world, sv);



	math::float3 sn = tree_.triangle_normal(index);
	math::float3 wn = math::transform_vector(transformation.rotation, sn);

//	std::cout << v  << " " << wn << std::endl;

	math::float3 axis = v - p;
	math::float3 dir = math::normalized(axis);

	float c = math::dot(wn, -dir);
	if (c <= 0.f) {
		sample.pdf = 0.f;
	} else {
		sample.wi = dir;
		sample.uv = tc;
		float sl = math::squared_length(axis);
		sample.t = std::sqrt(sl);
		sample.pdf = sl / (c * area);
	}

}

void Mesh::sample(uint32_t /*part*/, const entity::Composed_transformation& /*transformation*/, float /*area*/,
				  const math::float3& /*p*/, math::float2 /*uv*/, Sample& /*sample*/) const {}

void Mesh::sample(uint32_t /*part*/, const entity::Composed_transformation& /*transformation*/, float /*area*/,
				  const math::float3& /*p*/, const math::float3& /*wi*/, Sample& /*sample*/) const {}

float Mesh::pdf(uint32_t part, const entity::Composed_transformation& transformation, float area,
				const math::float3& p, const math::float3& wi, bool /*total_sphere*/,
				Node_stack& node_stack) const {
	math::Oray ray;
	ray.origin = math::transform_point(transformation.world_to_object, p);
	ray.set_direction(math::transform_vector(transformation.world_to_object, wi));
	ray.min_t = 0.f;
	ray.max_t = 10000.f;

	Intersection pi;
	if (tree_.intersect(ray, node_stack, pi)) {
		uint32_t shape_part = tree_.triangle_material_index(pi.index);
		if (part != shape_part) {
			return 0.f;
		}

		math::float3 sn = tree_.triangle_normal(pi.index);
		math::float3 wn = math::transform_vector(transformation.rotation, sn);

		float c = math::dot(wn, -wi);
		if (c <= 0.f) {
			return 0.f;
		}

		float sl = ray.max_t * ray.max_t;
		return sl / (c * area);
	}

	return 0.f;
}

float Mesh::area(uint32_t part, const math::float3& /*scale*/) const {
	return distributions_[part].distribution.integral();
}

bool Mesh::is_complex() const {
	return true;
}

bool Mesh::is_analytical() const {
	return false;
}

void Mesh::prepare_sampling(uint32_t part, const math::float3& scale) {
    distributions_[part].init(part, tree_, scale);
}

void Mesh::Distribution::init(uint32_t part, const Tree& tree, const math::float3& scale) {
	std::vector<float> areas;

	triangle_mapping.clear();

    for (uint32_t t = 0, len = tree.num_triangles(); t < len; ++t) {
        if (tree.triangle_material_index(t) == part) {
            areas.push_back(tree.triangle_area(t, scale));
			triangle_mapping.push_back(t);
		}
	}

	distribution.init(areas.data(), areas.size());
}

uint32_t Mesh::Distribution::sample(float r) const {
	return triangle_mapping[distribution.sample_discrete(r)];
}

}}}


