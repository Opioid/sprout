#include "triangle_morphable_mesh.hpp"
#include "triangle_intersection.hpp"
#include "triangle_morph_target_collection.hpp"
#include "triangle_primitive_mt.hpp"
#include "bvh/triangle_bvh_tree.inl"
#include "bvh/triangle_bvh_builder.inl"
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

namespace scene { namespace shape { namespace triangle {

Morphable_mesh::Morphable_mesh(std::shared_ptr<Morph_target_collection> collection, uint32_t num_parts) :
	collection_(collection), num_parts_(num_parts) {
	vertices_.resize(collection_->vertices(0).size());
}

void Morphable_mesh::init() {
	aabb_ = tree_.aabb();
}

uint32_t Morphable_mesh::num_parts() const {
	return num_parts_;
}

bool Morphable_mesh::intersect(const entity::Composed_transformation& transformation, math::Oray& ray,
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

		float bitangent_sign = tree_.triangle_bitangent_sign(pi.index);
		intersection.b = bitangent_sign * math::cross(intersection.n, intersection.t);
		intersection.uv = uv;
		intersection.part = tree_.triangle_material_index(pi.index);

		ray.max_t = tray.max_t;
		return true;
	}

	return false;
}

bool Morphable_mesh::intersect_p(const entity::Composed_transformation& transformation, const math::Oray& ray,
								 const math::float2& bounds, Node_stack& node_stack) const {
	math::Oray tray;
	tray.origin = math::transform_point(transformation.world_to_object, ray.origin);
	tray.set_direction(math::transform_vector(transformation.world_to_object, ray.direction));
	tray.min_t = ray.min_t;
	tray.max_t = ray.max_t;

	return tree_.intersect_p(tray, bounds, node_stack);
}

float Morphable_mesh::opacity(const entity::Composed_transformation& transformation, const math::Oray& ray,
							  const math::float2& bounds, Node_stack& node_stack,
							  const material::Materials& materials,
							  const image::texture::sampler::Sampler_2D& sampler) const {
	math::Oray tray;
	tray.origin = math::transform_point(transformation.world_to_object, ray.origin);
	tray.set_direction(math::transform_vector(transformation.world_to_object, ray.direction));
	tray.min_t = ray.min_t;
	tray.max_t = ray.max_t;

	return tree_.opacity(tray, bounds, node_stack, materials, sampler);
}

void Morphable_mesh::sample(uint32_t part, const entity::Composed_transformation& transformation, float area,
							const math::float3& p, const math::float3& /*n*/, bool /*total_sphere*/,
							sampler::Sampler& sampler, Sample& sample) const {
/*
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
		sample.uv = tc;
		float sl = math::squared_length(axis);
		sample.t = std::sqrt(sl);
		sample.pdf = sl / (c * area);
	}
	*/
}

void Morphable_mesh::sample(uint32_t /*part*/,
							const entity::Composed_transformation& /*transformation*/, float /*area*/,
							const math::float3& /*p*/, math::float2 /*uv*/, Sample& /*sample*/) const {}

void Morphable_mesh::sample(uint32_t /*part*/,
							const entity::Composed_transformation& /*transformation*/, float /*area*/,
							const math::float3& /*p*/, const math::float3& /*wi*/, Sample& /*sample*/) const {}

float Morphable_mesh::pdf(uint32_t /*part*/, const entity::Composed_transformation& /*transformation*/, float /*area*/,
						  const math::float3& /*p*/, const math::float3& /*wi*/,
						  bool /*total_sphere*/) const {
	return 1.f;
}

float Morphable_mesh::area(uint32_t /*part*/, const math::float3& /*scale*/) const {
	return 1.f;
}

bool Morphable_mesh::is_complex() const {
	return true;
}

bool Morphable_mesh::is_analytical() const {
	return false;
}

void Morphable_mesh::prepare_sampling(uint32_t /*part*/, const math::float3& /*scale*/) {}

Morphable_shape* Morphable_mesh::morphable_shape() {
	return this;
}

void Morphable_mesh::morph(uint32_t a, uint32_t b, float weight, thread::Pool& pool) {
//	std::cout << "morph " << a << " " << b << " " << weight << std::endl;

	collection_->morph(a, b, weight, pool, vertices_);

	bvh::Builder builder;
	builder.build<bvh::Data_MT>(tree_, collection_->triangles(), vertices_, 8);

	init();
}

}}}


